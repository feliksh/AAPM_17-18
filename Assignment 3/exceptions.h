
#ifndef MATRIXLIB_EXCEPTIONS_H
#define MATRIXLIB_EXCEPTIONS_H


#include <stdexcept>

void handle_exception(){
    try{ throw; }
    catch(const std::domain_error& e){
        std::cerr << "Exception dimensions: " << e.what() << std::endl;
    }
    catch(const std::logic_error& e ){
        std::cerr << "Exception logic error: " << e.what() << std::endl;
    }
    catch(const std::runtime_error& e){
        std::cerr << "Exception runtime error: " << e.what() << std::endl;
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Exception bad allocation: " << e.what() << std::endl;
    }
    catch(...){
        std::cerr << "Exception: rare\n";
    }
}

#endif //MATRIXLIB_EXCEPTIONS_H
