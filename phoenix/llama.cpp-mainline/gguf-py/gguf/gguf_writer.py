from __future__ import annotations

import logging
import os
import shutil
import struct
import tempfile
from enum import Enum, auto
from io import BufferedWriter
from typing import IO, Any, Callable, Sequence, Mapping
from string import ascii_letters, digits

import numpy as np

from .constants import (
    GGUF_DEFAULT_ALIGNMENT,
    GGUF_MAGIC,
    GGUF_VERSION,
    GGMLQuantizationType,
    GGUFEndian,
    GGUFValueType,
    Keys,
    RopeScalingType,
    PoolingType,
    TokenType,
)

logger = logging.getLogger(__name__)


class LazyTensor:
    data: Callable[[], np.ndarray[Any, Any]]
    # to avoid too deep recursion
    functions: list[Callable[[np.ndarray[Any, Any]], np.ndarray[Any, Any]]]
    dtype: np.dtype[Any]
    shape: tuple[int, ...]

    def __init__(self, data: Callable[[], np.ndarray[Any, Any]], *, dtype: type, shape: tuple[int, ...]):
        self.data = data
        self.functions = []
        self.dtype = np.dtype(dtype)
        self.shape = shape

    def astype(self, dtype: type, **kwargs) -> LazyTensor:
        self.functions.append(lambda n: n.astype(dtype, **kwargs))
        self.dtype = np.dtype(dtype)
        return self

    @property
    def nbytes(self) -> int:
        size = 1
        for n in self.shape:
            size *= n
        return size * self.dtype.itemsize

    def tofile(self, *args, **kwargs) -> None:
        data = self.data()
        for f in self.functions:
            data = f(data)
        assert data.shape == self.shape
        assert data.dtype == self.dtype
        assert data.nbytes == self.nbytes
        self.functions = []
        self.data = lambda: data
        data.tofile(*args, **kwargs)

    def byteswap(self, *args, **kwargs) -> LazyTensor:
        self.functions.append(lambda n: n.byteswap(*args, **kwargs))
        return self


class WriterState(Enum):
    EMPTY   = auto()
    HEADER  = auto()
    KV_DATA = auto()
    TI_DATA = auto()


class GGUFWriter:
    fout: BufferedWriter
    temp_file: tempfile.SpooledTemporaryFile[bytes] | None
    tensors: list[np.ndarray[Any, Any] | LazyTensor]
    _simple_value_packing = {
        GGUFValueType.UINT8:   "B",
        GGUFValueType.INT8:    "b",
        GGUFValueType.UINT16:  "H",
        GGUFValueType.INT16:   "h",
        GGUFValueType.UINT32:  "I",
        GGUFValueType.INT32:   "i",
        GGUFValueType.FLOAT32: "f",
        GGUFValueType.UINT64:  "Q",
        GGUFValueType.INT64:   "q",
        GGUFValueType.FLOAT64: "d",
        GGUFValueType.BOOL:    "?",
    }

    def __init__(
        self, path: os.PathLike[str] | str, arch: str, use_temp_file: bool = True,
        endianess: GGUFEndian = GGUFEndian.LITTLE,
    ):
        self.fout = open(path, "wb")
        self.arch = arch
        self.endianess = endianess
        self.offset_tensor = 0
        self.data_alignment = GGUF_DEFAULT_ALIGNMENT
        self.kv_data = bytearray()
        self.kv_data_count = 0
        self.ti_data = bytearray()
        self.ti_data_count = 0
        self.ti_names = set()
        self.use_temp_file = use_temp_file
        self.temp_file = None
        self.tensors = []
        logger.info("gguf: This GGUF file is for {0} Endian only".format(
            "Big" if self.endianess == GGUFEndian.BIG else "Little",
        ))
        self.state = WriterState.EMPTY

        self.add_architecture()

    def write_header_to_file(self) -> None:
        if self.state is not WriterState.EMPTY:
            raise ValueError(f'Expected output file to be empty, got {self.state}')

        self._write_packed("<I", GGUF_MAGIC, skip_pack_prefix = True)
        self._write_packed("I", GGUF_VERSION)
        self._write_packed("Q", self.ti_data_count)
        self._write_packed("Q", self.kv_data_count)
        self.flush()
        self.state = WriterState.HEADER

    def write_kv_data_to_file(self) -> None:
        if self.state is not WriterState.HEADER:
            raise ValueError(f'Expected output file to contain the header, got {self.state}')

        self.fout.write(self.kv_data)
        self.flush()
        self.state = WriterState.KV_DATA

    def write_ti_data_to_file(self) -> None:
        if self.state is not WriterState.KV_DATA:
            raise ValueError(f'Expected output file to contain KV data, got {self.state}')

        self.fout.write(self.ti_data)
        self.flush()
        self.state = WriterState.TI_DATA

    def add_key(self, key: str) -> None:
        self.add_val(key, GGUFValueType.STRING, add_vtype=False)

    def add_uint8(self, key: str, val: int) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.UINT8)

    def add_int8(self, key: str, val: int) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.INT8)

    def add_uint16(self, key: str, val: int) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.UINT16)

    def add_int16(self, key: str, val: int) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.INT16)

    def add_uint32(self, key: str, val: int) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.UINT32)

    def add_int32(self, key: str, val: int) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.INT32)

    def add_float32(self, key: str, val: float) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.FLOAT32)

    def add_uint64(self, key: str, val: int) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.UINT64)

    def add_int64(self, key: str, val: int) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.INT64)

    def add_float64(self, key: str, val: float) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.FLOAT64)

    def add_bool(self, key: str, val: bool) -> None:
        self.add_key(key)
        self.add_val(val, GGUFValueType.BOOL)

    def add_string(self, key: str, val: str) -> None:
        if not val:
            return
        self.add_key(key)
        self.add_val(val, GGUFValueType.STRING)

    def add_array(self, key: str, val: Sequence[Any]) -> None:
        if not isinstance(val, Sequence):
            raise ValueError("Value must be a sequence for array type")

        self.add_key(key)
        self.add_val(val, GGUFValueType.ARRAY)

    def add_val(self, val: Any, vtype: GGUFValueType | None = None, add_vtype: bool = True) -> None:
        if vtype is None:
            vtype = GGUFValueType.get_type(val)

        if add_vtype:
            self.kv_data += self._pack("I", vtype)
            self.kv_data_count += 1

        pack_fmt = self._simple_value_packing.get(vtype)
        if pack_fmt is not None:
            self.kv_data += self._pack(pack_fmt, val, skip_pack_prefix = vtype == GGUFValueType.BOOL)
        elif vtype == GGUFValueType.STRING:
            encoded_val = val.encode("utf-8") if isinstance(val, str) else val
            self.kv_data += self._pack("Q", len(encoded_val))
            self.kv_data += encoded_val
        elif vtype == GGUFValueType.ARRAY and isinstance(val, Sequence) and val:
            ltype = GGUFValueType.get_type(val[0])
            if not all(GGUFValueType.get_type(i) is ltype for i in val[1:]):
                raise ValueError("All items in a GGUF array should be of the same type")
            self.kv_data += self._pack("I", ltype)
            self.kv_data += self._pack("Q", len(val))
            for item in val:
                self.add_val(item, add_vtype=False)
        else:
            raise ValueError("Invalid GGUF metadata value type or value")

    @staticmethod
    def ggml_pad(x: int, n: int) -> int:
        return ((x + n - 1) // n) * n

    def add_tensor_info(
        self, name: str, tensor_shape: Sequence[int], tensor_dtype: np.dtype[np.float16] | np.dtype[np.float32],
        tensor_nbytes: int, raw_dtype: GGMLQuantizationType | None = None,
    ) -> None:
        if self.state is not WriterState.EMPTY:
            raise ValueError(f'Expected output file to be empty, got {self.state}')

        if name in self.ti_names:
            raise ValueError(f'Duplicated tensor name {name}')
        self.ti_names.add(name)

        encoded_name = name.encode("utf-8")
        self.ti_data += self._pack("Q", len(encoded_name))
        self.ti_data += encoded_name
        n_dims = len(tensor_shape)
        self.ti_data += self._pack("I", n_dims)
        for i in range(n_dims):
            self.ti_data += self._pack("Q", tensor_shape[n_dims - 1 - i])
        if raw_dtype is None:
            if tensor_dtype == np.float16:
                dtype = GGMLQuantizationType.F16
            elif tensor_dtype == np.float32:
                dtype = GGMLQuantizationType.F32
            elif tensor_dtype == np.float64:
                dtype = GGMLQuantizationType.F64
            elif tensor_dtype == np.int8:
                dtype = GGMLQuantizationType.I8
            elif tensor_dtype == np.int16:
                dtype = GGMLQuantizationType.I16
            elif tensor_dtype == np.int32:
                dtype = GGMLQuantizationType.I32
            elif tensor_dtype == np.int64:
                dtype = GGMLQuantizationType.I64
            else:
                raise ValueError("Only F16, F32, F64, I8, I16, I32, I64 tensors are supported for now")
        else:
            dtype = raw_dtype
        self.ti_data += self._pack("I", dtype)
        self.ti_data += self._pack("Q", self.offset_tensor)
        self.offset_tensor += GGUFWriter.ggml_pad(tensor_nbytes, self.data_alignment)
        self.ti_data_count += 1

    def add_tensor(
        self, name: str, tensor: np.ndarray[Any, Any] | LazyTensor, raw_shape: Sequence[int] | None = None,
        raw_dtype: GGMLQuantizationType | None = None,
    ) -> None:
        if self.endianess == GGUFEndian.BIG:
            tensor.byteswap(inplace=True)
        if self.use_temp_file and self.temp_file is None:
            fp = tempfile.SpooledTemporaryFile(mode="w+b", max_size=256 * 1024 * 1024)
            fp.seek(0)
            self.temp_file = fp

        shape: Sequence[int] = raw_shape if raw_shape is not None else tensor.shape
        self.add_tensor_info(name, shape, tensor.dtype, tensor.nbytes, raw_dtype = raw_dtype)

        if self.temp_file is None:
            self.tensors.append(tensor)
            return

        tensor.tofile(self.temp_file)
        self.write_padding(self.temp_file, tensor.nbytes)

    def write_padding(self, fp: IO[bytes], n: int, align: int | None = None) -> None:
        pad = GGUFWriter.ggml_pad(n, align if align is not None else self.data_alignment) - n
        if pad != 0:
            fp.write(bytes([0] * pad))

    def write_tensor_data(self, tensor: np.ndarray[Any, Any] | LazyTensor) -> None:
        if self.state is not WriterState.TI_DATA:
            raise ValueError(f'Expected output file to contain tensor info, got {self.state}')

        if self.endianess == GGUFEndian.BIG:
            tensor.byteswap(inplace=True)
        self.write_padding(self.fout, self.fout.tell())
        tensor.tofile(self.fout)
        self.write_padding(self.fout, tensor.nbytes)

    def write_tensors_to_file(self, *, progress: bool = False) -> None:
        self.write_ti_data_to_file()

        self.write_padding(self.fout, self.fout.tell())

        if self.temp_file is None:
            self.tensors.reverse()  # to pop from the "beginning" in constant time

            if progress:
                from tqdm import tqdm

                total_bytes = sum(t.nbytes for t in self.tensors)

                bar = tqdm(desc="Writing", total=total_bytes, unit="byte", unit_scale=True)

                while True:
                    try:
                        tensor = self.tensors.pop()
                    except IndexError:
                        break
                    tensor.tofile(self.fout)
                    bar.update(tensor.nbytes)
                    self.write_padding(self.fout, tensor.nbytes)
                return
            while True:
                try:
                    tensor = self.tensors.pop()
                except IndexError:
                    break
                tensor.tofile(self.fout)
                self.write_padding(self.fout, tensor.nbytes)
            return

        self.temp_file.seek(0)

        shutil.copyfileobj(self.temp_file, self.fout)
        self.flush()
        self.temp_file.close()

    def flush(self) -> None:
        self.fout.flush()

    def close(self) -> None:
        self.fout.close()

    def add_architecture(self) -> None:
        self.add_string(Keys.General.ARCHITECTURE, self.arch)

    def add_author(self, author: str) -> None:
        self.add_string(Keys.General.AUTHOR, author)

    def add_version(self, version: str) -> None:
        self.add_string(Keys.General.VERSION, version)

    def add_tensor_data_layout(self, layout: str) -> None:
        self.add_string(Keys.LLM.TENSOR_DATA_LAYOUT.format(arch=self.arch), layout)

    def add_url(self, url: str) -> None:
        self.add_string(Keys.General.URL, url)

    def add_description(self, description: str) -> None:
        self.add_string(Keys.General.DESCRIPTION, description)

    def add_licence(self, licence: str) -> None:
        self.add_string(Keys.General.LICENSE, licence)

    def add_source_url(self, url: str) -> None:
        self.add_string(Keys.General.SOURCE_URL, url)

    def add_source_hf_repo(self, repo: str) -> None:
        self.add_string(Keys.General.SOURCE_HF_REPO, repo)

    def add_file_type(self, ftype: int) -> None:
        self.add_uint32(Keys.General.FILE_TYPE, ftype)

    def add_name(self, name: str) -> None:
        self.add_string(Keys.General.NAME, name)

    def add_quantization_version(self, quantization_version: GGMLQuantizationType) -> None:
        self.add_uint32(
            Keys.General.QUANTIZATION_VERSION, quantization_version)

    def add_custom_alignment(self, alignment: int) -> None:
        self.data_alignment = alignment
        self.add_uint32(Keys.General.ALIGNMENT, alignment)

    def add_vocab_size(self, size: int) -> None:
        self.add_uint32(Keys.LLM.VOCAB_SIZE.format(arch=self.arch), size)

    def add_context_length(self, length: int) -> None:
        self.add_uint32(Keys.LLM.CONTEXT_LENGTH.format(arch=self.arch), length)

    def add_embedding_length(self, length: int) -> None:
        self.add_uint32(Keys.LLM.EMBEDDING_LENGTH.format(arch=self.arch), length)

    def add_block_count(self, length: int) -> None:
        self.add_uint32(Keys.LLM.BLOCK_COUNT.format(arch=self.arch), length)

    def add_feed_forward_length(self, length: int) -> None:
        self.add_uint32(Keys.LLM.FEED_FORWARD_LENGTH.format(arch=self.arch), length)

    def add_parallel_residual(self, use: bool) -> None:
        self.add_bool(Keys.LLM.USE_PARALLEL_RESIDUAL.format(arch=self.arch), use)

    def add_head_count(self, count: int) -> None:
        self.add_uint32(Keys.Attention.HEAD_COUNT.format(arch=self.arch), count)

    def add_head_count_kv(self, count: int) -> None:
        self.add_uint32(Keys.Attention.HEAD_COUNT_KV.format(arch=self.arch), count)

    def add_key_length(self, length: int) -> None:
        self.add_uint32(Keys.Attention.KEY_LENGTH.format(arch=self.arch), length)

    def add_value_length(self, length: int) -> None:
        self.add_uint32(Keys.Attention.VALUE_LENGTH.format(arch=self.arch), length)

    def add_max_alibi_bias(self, bias: float) -> None:
        self.add_float32(Keys.Attention.MAX_ALIBI_BIAS.format(arch=self.arch), bias)

    def add_clamp_kqv(self, value: float) -> None:
        self.add_float32(Keys.Attention.CLAMP_KQV.format(arch=self.arch), value)

    def add_logit_scale(self, value: float) -> None:
        self.add_float32(Keys.LLM.LOGIT_SCALE.format(arch=self.arch), value)

    def add_expert_count(self, count: int) -> None:
        self.add_uint32(Keys.LLM.EXPERT_COUNT.format(arch=self.arch), count)

    def add_expert_used_count(self, count: int) -> None:
        self.add_uint32(Keys.LLM.EXPERT_USED_COUNT.format(arch=self.arch), count)

    def add_layer_norm_eps(self, value: float) -> None:
        self.add_float32(Keys.Attention.LAYERNORM_EPS.format(arch=self.arch), value)

    def add_layer_norm_rms_eps(self, value: float) -> None:
        self.add_float32(Keys.Attention.LAYERNORM_RMS_EPS.format(arch=self.arch), value)

    def add_causal_attention(self, value: bool) -> None:
        self.add_bool(Keys.Attention.CAUSAL.format(arch=self.arch), value)

    def add_pooling_type(self, value: PoolingType) -> None:
        self.add_uint32(Keys.LLM.POOLING_TYPE.format(arch=self.arch), value.value)

    def add_rope_dimension_count(self, count: int) -> None:
        self.add_uint32(Keys.Rope.DIMENSION_COUNT.format(arch=self.arch), count)

    def add_rope_freq_base(self, value: float) -> None:
        self.add_float32(Keys.Rope.FREQ_BASE.format(arch=self.arch), value)

    def add_rope_scaling_type(self, value: RopeScalingType) -> None:
        self.add_string(Keys.Rope.SCALING_TYPE.format(arch=self.arch), value.value)

    def add_rope_scaling_factor(self, value: float) -> None:
        self.add_float32(Keys.Rope.SCALING_FACTOR.format(arch=self.arch), value)

    def add_rope_scaling_orig_ctx_len(self, value: int) -> None:
        self.add_uint32(Keys.Rope.SCALING_ORIG_CTX_LEN.format(arch=self.arch), value)

    def add_rope_scaling_finetuned(self, value: bool) -> None:
        self.add_bool(Keys.Rope.SCALING_FINETUNED.format(arch=self.arch), value)

    def add_ssm_conv_kernel(self, value: int) -> None:
        self.add_uint32(Keys.SSM.CONV_KERNEL.format(arch=self.arch), value)

    def add_ssm_inner_size(self, value: int) -> None:
        self.add_uint32(Keys.SSM.INNER_SIZE.format(arch=self.arch), value)

    def add_ssm_state_size(self, value: int) -> None:
        self.add_uint32(Keys.SSM.STATE_SIZE.format(arch=self.arch), value)

    def add_ssm_time_step_rank(self, value: int) -> None:
        self.add_uint32(Keys.SSM.TIME_STEP_RANK.format(arch=self.arch), value)

    def add_tokenizer_model(self, model: str) -> None:
        self.add_string(Keys.Tokenizer.MODEL, model)

    def add_tokenizer_pre(self, pre: str) -> None:
        self.add_string(Keys.Tokenizer.PRE, pre)

    def add_token_list(self, tokens: Sequence[str] | Sequence[bytes] | Sequence[bytearray]) -> None:
        self.add_array(Keys.Tokenizer.LIST, tokens)

    def add_token_merges(self, merges: Sequence[str] | Sequence[bytes] | Sequence[bytearray]) -> None:
        self.add_array(Keys.Tokenizer.MERGES, merges)

    def add_token_types(self, types: Sequence[TokenType] | Sequence[int]) -> None:
        self.add_array(Keys.Tokenizer.TOKEN_TYPE, types)

    def add_token_type_count(self, value: int) -> None:
        self.add_uint32(Keys.Tokenizer.TOKEN_TYPE_COUNT, value)

    def add_token_scores(self, scores: Sequence[float]) -> None:
        self.add_array(Keys.Tokenizer.SCORES, scores)

    def add_bos_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.BOS_ID, id)

    def add_eos_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.EOS_ID, id)

    def add_unk_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.UNK_ID, id)

    def add_sep_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.SEP_ID, id)

    def add_pad_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.PAD_ID, id)

    def add_cls_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.CLS_ID, id)

    def add_mask_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.MASK_ID, id)

    def add_add_bos_token(self, value: bool) -> None:
        self.add_bool(Keys.Tokenizer.ADD_BOS, value)

    def add_add_eos_token(self, value: bool) -> None:
        self.add_bool(Keys.Tokenizer.ADD_EOS, value)

    def add_add_space_prefix(self, value: bool) -> None:
        self.add_bool(Keys.Tokenizer.ADD_PREFIX, value)

    def add_chat_template(self, value: str | Sequence[Mapping[str, str]]) -> None:
        if not isinstance(value, str):
            template_default = None
            template_names = set()

            for choice in value:
                name = choice.get('name', '')
                template = choice.get('template')

                # Allowing non-alphanumerical characters in template name is probably not a good idea, so filter it
                name = ''.join((c if c in ascii_letters + digits else '_' for c in name))

                if name and template is not None:
                    if name == 'default':
                        template_default = template
                    else:
                        template_names.add(name)
                        self.add_string(Keys.Tokenizer.CHAT_TEMPLATE_N.format(name=name), template)

            if template_names:
                self.add_array(Keys.Tokenizer.CHAT_TEMPLATES, list(template_names))

            if template_default is None:
                return

            value = template_default

        self.add_string(Keys.Tokenizer.CHAT_TEMPLATE, value)

    def add_prefix_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.PREFIX_ID, id)

    def add_suffix_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.SUFFIX_ID, id)

    def add_middle_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.MIDDLE_ID, id)

    def add_eot_token_id(self, id: int) -> None:
        self.add_uint32(Keys.Tokenizer.EOT_ID, id)

    def _pack(self, fmt: str, value: Any, skip_pack_prefix: bool = False) -> bytes:
        pack_prefix = ''
        if not skip_pack_prefix:
            pack_prefix = '<' if self.endianess == GGUFEndian.LITTLE else '>'
        return struct.pack(f'{pack_prefix}{fmt}', value)

    def _write_packed(self, fmt: str, value: Any, skip_pack_prefix: bool = False) -> None:
        self.fout.write(self._pack(fmt, value, skip_pack_prefix))