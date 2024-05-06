#include "ollama.hpp"
#include "json.hpp"

#include <iostream>
#include <string>
#include <functional>
#include <thread>
#include <chrono>

std::string output;

void on_receive_token(const std::string& token, bool done)
{
    std::cout << token << std::flush;
    if (done) std::cout << std::endl;
    output +=token;
}

void on_receive_json(const json& token)
{
    std::cout << token.dump() << std::endl;
}

int main()
{

    // Optional. By default, the server URL is set to http://localhost:11434. Use this function if you need to point to a different URL.
    ollama::setServerURL("http://localhost:11434");    

    // Optional. Set the read and write timeouts in seconds for receiving from and sending data to ollama.
    // If you have a large model with a long response time you may need to increase these.
    ollama::setReadTimeout(30);
    ollama::setWriteTimeout(30);

    // Check to see whether the ollama server is running.
    std::cout << ollama::is_running() << std::endl;
    
    // Get the version of the ollama server
    std::cout << ollama::get_version() << std::endl;

    // Optionally send a request to ollama to load a model into memory.
    // This will occur automatically during generation but this allows you to preload a model before using it.
    bool model_loaded = ollama::load_model("llama3");

    // Perform a simple generation to a string by specifying a model and a prompt. The response will be returned as one string without streaming the reply.
    std::cout << ollama::generate("llama3", "Why is the sky blue?") << std::endl;
    
    // Set options to include with use of the model. These should be specified in JSON.
    json options;
    options["options"]["top_k"] = 20;

    // Perform a simple generation which includes model options.
    std::cout << ollama::generate("llama3", "Why is the sky blue?", options) << std::endl;

    // Define a callback function to receive a streaming reply from the server. This will allow you to perform an action on receiving each token.
    // Socket communication is blocking so this will still block the main thread.
    std::function<void(const std::string&, bool)> callback = on_receive_token;    
    ollama::generate("llama3", "Why is the sky blue?", callback);

    // You can launch the generation in a thread with a callback to use it asynchronously.
    std::thread new_thread( [callback]{ ollama::generate("llama3", "Why is the sky blue?", callback); } );

    // Prevent the main thread from exiting while we wait from an asynchronous response.
    while(1) { std::this_thread::sleep_for(std::chrono::microseconds(100) ); }
    
    // If you don't want to use the static singleton defined in the namespace, you can create an Ollama instance itself.
    // This is helpful if you have multiple Ollama servers or need custom control over the object.
    Ollama my_ollama_server("http://localhost:11434");

    // You can use all of the same functions from this instanced version of the class.
    std::cout << my_ollama_server.generate("llama3", "Why is the sky blue?") << std::endl;
}