#include "header.h"
#include "../gptnerd_backend/llmodel_c.h"
#include "utils.h"
#include "parse_json.h"


//----------------------------------ANIMATION----------------------------------//
std::atomic<bool> stop_display{false}; 

void display_frames() {
    const char* frames[] = {".", ":", "'", ":"};
    int frame_index = 0;
    ConsoleState con_st;
    con_st.use_color = true;
    while (!stop_display) {
        set_console_color(con_st, PROMPT);
        std::cerr << "\r" << frames[frame_index % 4] << std::flush;
        frame_index++;
        set_console_color(con_st, DEFAULT);
        if (!stop_display){
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cerr << "\r" << " " << std::flush;
            std::cerr << "\r" << std::flush;
        }
    }
}

void display_loading() {
    while (!stop_display) {
        for (int i = 0; i < 14; i++) {
            fprintf(stdout, ".");
            fflush(stdout);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            if (stop_display) { break; }
        }
        std::cout << "\r" << "               " << "\r" << std::flush;
    }
    std::cout << "\r" << " " << std::flush;
}

//----------------------------------ANIMATION----------------------------------//

//**********************************************************

//------------------------------CHAT FUNCTIONS------------------------------//

std::string get_input(ConsoleState& con_st, std::string& input) {
    set_console_color(con_st, USER_INPUT);

    std::cout << "\n> ";
    std::getline(std::cin, input);
    std::istringstream iss(input);
    
    return input;
}

std::string hashstring = "";
std::string answer = "";
//------------------------------CHAT FUNCTIONS------------------------------//

//**********************************************************

//---------------------PROMPT LAMBDA FUNCTIONS---------------------//
auto prompt_callback = [](int32_t token_id) {
    // You can handle prompt here if needed
    return true;
    };


auto response_callback = [](int32_t token_id, const char* responsechars) {

    if (!(responsechars == nullptr || responsechars[0] == '\0')) {
        // stop the animation, printing response
        if (stop_display == false) {
            stop_display = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cerr << "\r" << " " << std::flush;
            std::cerr << "\r" << std::flush;
            if (answer != "") { std::cout << answer; }
        }

        std::cout << responsechars << std::flush;
        answer += responsechars;
    }

    return true;
    };

auto recalculate_callback = [](bool is_recalculating) {
    // You can handle recalculation requests here if needed
    return is_recalculating;
    };
//---------------------PROMPT LAMBDA FUNCTIONS---------------------//

//**********************************************************

//-----------------------------LOAD THE MODEL------------------------------//
 
bool load_model(chatParams& params, llmodel_model& model, std::future<void>& future) {
    //animation
    stop_display = true;
    if (params.use_animation) { stop_display = false; future = std::async(std::launch::async, display_loading); }

    //handle stderr for now
    //this is just to prevent printing unnecessary details during model loading.
    int stderr_copy = dup(fileno(stderr));
#ifdef _WIN32
    std::freopen("NUL", "w", stderr);
#else
    std::freopen("/dev/null", "w", stderr);
#endif

    std::cout << "\r" << APPNAME << ": loading " << params.model.c_str() << std::endl;

    //bring back stderr for now
    dup2(stderr_copy, fileno(stderr));
    close(stderr_copy);

    //check if model is loaded
    auto check_model = llmodel_loadModel(model, params.model.c_str());

    if (check_model == false) {
        if (params.use_animation) {
            stop_display = true;
            future.wait();
            stop_display = false;
        }
        std::cerr << "Error loading: " << params.model.c_str() << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        std::cin.get();
        return false;
    }
    else {
        if (params.use_animation) {
            stop_display = true;
            future.wait();
        }
        std::cout << "\r" << APPNAME << ": done loading!" << std::flush;
    }
    return true;
 }

//-----------------------------LOAD THE MODEL------------------------------//

//**********************************************************

//-------------------------------MAIN PROGRAM-------------------------------//

int main(int argc, char* argv[]) {

    ConsoleState con_st;
    con_st.use_color = true;

    set_console_color(con_st, BOLD);std::cout << APPNAME;set_console_color(con_st, DEFAULT);
    set_console_color(con_st, PROMPT);std::cout << " (v. " << VERSION << ")\n";set_console_color(con_st, DEFAULT);

    check_avx_support_at_startup();

    std::cout << "Enter your model path:\n";
    std::string path;
    set_console_color(con_st, USER_INPUT);
    std::getline(std::cin, path);
    set_console_color(con_st, DEFAULT);

    chatParams params;
    params.model = path;

    //***get all parameters from cli arguments or json
    parse_params(argc, argv, params);

    //***convert the default model path into Windows format if on WIN32
#ifdef _WIN32
    std::filesystem::path p(params.model);
    params.model = p.make_preferred().string();
#endif
    
    //***Create a prompt_context and copy all params from chatParams to prompt_context
    llmodel_prompt_context prompt_context = {
     .logits = params.logits,
     .logits_size = params.logits_size,
     .tokens = params.tokens,
     .tokens_size = params.tokens_size,
     .n_past = params.n_past,
     .n_ctx = params.n_ctx,
     .n_predict = params.n_predict,
     .top_k = params.top_k,
     .top_p = params.top_p,
     .temp = params.temp,
     .n_batch = params.n_batch,
     .repeat_penalty = params.repeat_penalty,  
     .repeat_last_n = params.repeat_last_n,
     .context_erase = params.context_erase,
    }; 

    //Subprocess signal handling
    #ifdef _WIN32
        SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
    #else
        signal(SIGHUP, handle_sighup);
    #endif
 
    std::future<void> future;
    llmodel_model model = llmodel_model_create(params.model.c_str());

    //load model
    int check_model = load_model(params, model, future);
    if (check_model == false)return 0;
    set_console_color(con_st, PROMPT);std::cout << "\n" << params.prompt.c_str() << std::endl;set_console_color(con_st, DEFAULT);

    //***load prompt template from file instead
    if (params.load_template != "") {
        std::tie(params.default_prefix, params.default_header, params.default_footer) = read_prompt_template_file(params.load_template);
    }
    
    //***load chat log from a file
    if (params.load_log != "") {
    	if (params.prompt == "") {
        	params.prompt = params.default_prefix + read_chat_log(params.load_log) + params.default_header;
        } else {
        	params.prompt = params.default_prefix + read_chat_log(params.load_log) + params.default_header + params.prompt;
        }
    } else {
    	params.prompt = params.default_prefix + params.default_header + params.prompt;
    }
    
    llmodel_setThreadCount(model, params.n_threads);

    std::string input = "";

    //***main chat loop.
    if (!params.no_interactive && !sighup_received) {
        input = get_input(con_st, input);

        //Interactive mode. We have a prompt.
        if (params.prompt != "") {
            if (params.use_animation){ stop_display = false; future = std::async(std::launch::async, display_frames); }
            if (params.b_token != ""){answer = answer + params.b_token; if(!params.use_animation) {std::cout << params.b_token;} }
            llmodel_prompt(model, (params.prompt + " " + input + params.default_footer).c_str(),prompt_callback, response_callback, recalculate_callback, &prompt_context);
            if (params.e_token != ""){std::cout << params.e_token; answer = answer + params.e_token; }
            if (params.use_animation){ stop_display = true; future.wait(); stop_display = false; }
            if (params.save_log != ""){ save_chat_log(params.save_log, (params.prompt + " " + input + params.default_footer).c_str(), answer.c_str()); }
        //Interactive mode. Else get prompt from input.
        }
        else {
            if (params.use_animation){ stop_display = false; future = std::async(std::launch::async, display_frames); }
            if (params.b_token != ""){answer = answer + params.b_token; if(!params.use_animation) {std::cout << params.b_token;} }
            llmodel_prompt(model, (params.default_prefix + params.default_header + input + params.default_footer).c_str(),prompt_callback, response_callback, recalculate_callback, &prompt_context);
            if (params.e_token != ""){std::cout << params.e_token; answer = answer + params.e_token; }
            if (params.use_animation){ stop_display = true; future.wait(); stop_display = false; }
            if (params.save_log != ""){ save_chat_log(params.save_log, (params.default_prefix + params.default_header + input + params.default_footer).c_str(), answer.c_str()); }
        }
        //Interactive and continuous mode. Get prompt from input.

        while (!params.run_once && !sighup_received) {
            answer = ""; //New prompt. We stored previous answer in memory so clear it.
            input = get_input(con_st, input);
            if (params.use_animation){ stop_display = false; future = std::async(std::launch::async, display_frames); }
            if (params.b_token != ""){answer = answer + params.b_token; if(!params.use_animation) {std::cout << params.b_token;} }
            llmodel_prompt(model, (params.default_prefix + params.default_header + input + params.default_footer).c_str(), prompt_callback, response_callback, recalculate_callback, &prompt_context);
            if (params.e_token != ""){std::cout << params.e_token; answer = answer + params.e_token; }
            if (params.use_animation){ stop_display = true; future.wait(); stop_display = false; }
            if (params.save_log != ""){ save_chat_log(params.save_log, (params.default_prefix + params.default_header + input + params.default_footer).c_str(), answer.c_str()); }
          }
    //No-interactive mode. Get the answer once from prompt and print it.
    } else {
        if (params.use_animation){ stop_display = false; future = std::async(std::launch::async, display_frames); }
        if (params.b_token != ""){answer = answer + params.b_token; if(!params.use_animation) {std::cout << params.b_token;} }
        llmodel_prompt(model, (params.prompt + params.default_footer).c_str(),prompt_callback, response_callback, recalculate_callback, &prompt_context);
        if (params.e_token != ""){std::cout << params.e_token; answer = answer + params.e_token; }
        if (params.use_animation){ stop_display = true; future.wait(); stop_display = false; }
        if (params.save_log != ""){ save_chat_log(params.save_log, (params.prompt + params.default_footer).c_str(), answer.c_str()); }
        std::cout << std::endl;
    }


    set_console_color(con_st, DEFAULT);
    llmodel_model_destroy(model);
    return 0;
}
//-------------------------------MAIN PROGRAM-------------------------------//