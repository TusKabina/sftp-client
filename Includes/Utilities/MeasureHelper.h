#ifndef SFTP_CLIENT_MEASURE_HELPER_H
#define SFTP_CLIENT_MEASURE_HELPER_H

#include <iostream>
#include <chrono>
#include <functional>
#include <type_traits>
#include "Utilities/Logger.h"

namespace MeasureHelper {

    template <typename Func, typename... Args>
    std::enable_if_t<std::is_void_v<std::invoke_result_t<Func, Args...>>>
    inline Time(Func&& func, Args&&... args) {
        auto start = std::chrono::high_resolution_clock::now();

        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "Execution time: " << duration.count() << " ms\n";
        logger().debug() << "Execution time: " << duration.count() << " ms";
    }

    template <typename Func, typename... Args>
    inline auto Time(Func&& func, Args&&... args) -> std::enable_if_t<!std::is_void_v<std::invoke_result_t<Func, Args...>>, std::invoke_result_t<Func, Args...>> {
        auto start = std::chrono::high_resolution_clock::now();  

   
        auto result = std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

        auto end = std::chrono::high_resolution_clock::now();  
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "Execution time: " << duration.count() << " ms\n";
		logger().debug() << "Execution time: " << duration.count() << " ms";

        return result;
    }

    inline auto logDuration = [](const std::string& section, auto start, auto end) {
        std::cout << section << " took: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << " ms" << std::endl;

        logger().debug() << section << " took: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << " ms";
    };
}
#endif // SFTP_CLIENT_MEASURE_HELPER_H