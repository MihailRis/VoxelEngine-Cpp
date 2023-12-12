#include "quickjspp.hpp"


void println(qjs::rest<std::string> args) {
    for (auto const & arg : args) std::cout << arg << " ";
    std::cout << "\n";
}

void jstest(){
    qjs::Runtime runtime;
    qjs::Context context(runtime);
try
    {
        auto& module = context.addModule("VoxelEngine");
        module.function<&println>("println");

        // import module
        context.eval(R"xxx(
            import * as game from 'VoxelEngine';
            globalThis.game = game;
        )xxx", "<import>", JS_EVAL_TYPE_MODULE);

        // evaluate js code
        context.eval(R"xxx(
            test("Javascript");
            function test(str) {
              game.println("Hello from " + str + "!");
            }
        )xxx");
    }
    catch(qjs::exception)
    {
        auto exc = context.getException();
        std::cerr << (std::string) exc << std::endl;
        if((bool) exc["stack"])
            std::cerr << (std::string) exc["stack"] << std::endl;
    }

}