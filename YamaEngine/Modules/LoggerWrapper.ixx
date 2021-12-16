export module Logger;

import "loguru.hpp"; //https://emilk.github.io/loguru/index.html#loguru
export import <source_location>;

// std::format will be used for logger to take in the string.
// std::format also requires the working draft C++
export import <format>;
export import <string>;
/**
* @brief Mode for handling text log files.
*/
export enum FileWriteMode
{
	Overwrite = loguru::FileMode::Truncate,
	Append = loguru::FileMode::Append
};

/**
 * @brief Verbosity levels for the logger.
*/
export enum Verbosity
{
	LGR_INFO = loguru::NamedVerbosity::Verbosity_INFO,
	LGR_WARNING = loguru::NamedVerbosity::Verbosity_WARNING,
	LGR_ERROR = loguru::NamedVerbosity::Verbosity_ERROR
};

/**
 * @brief Initialize the logger.
 * Will use the arguments passed into to the program to determine verbosity, etc.
 * @param argc
 * @param argv
*/
export inline void InitLogger(int& argc, char* argv[])
{
	if (argc && argv)
		loguru::init(argc, argv);
}

/**
 * @brief Set verbosity of the logger.
 * @param verbosityLevel
*/
export inline void SetVerbosity(Verbosity const& verbosityLevel = LGR_INFO)
{
	loguru::g_stderr_verbosity = (loguru::NamedVerbosity)verbosityLevel;
}

/**
 * @brief Add a file that logger will write to.
 * @param path Path to the logger. If elements do not exist - they will be created.
 * @param fileWriteMode
 * @param verbosityLevel
*/
export inline void AddLogFile(std::string const& path, FileWriteMode const& fileWriteMode, Verbosity const& verbosityLevel)
{
	loguru::add_file(path.c_str(), (loguru::FileMode)fileWriteMode, (loguru::Verbosity)verbosityLevel);
}

/**
 * @brief Log a message.
 * @param verbosity
 * @param message
 * @param current Used for the context of the place that logs.
	Should not be overwritten unless logging about some other place in the code.
*/
export inline void Log(Verbosity const& verbosity, std::string const& message)
{
	// https://developercommunity.visualstudio.com/t/cannot-use-stdsource-location-in-c-modules-applies/1436152
	// Would get error C7595: 'std::source_location::current': call to immediate function is not a constant expression
	// Even with __forceinline the captured location was of the wrapper, most likely because of the static current() use
	auto const& location = std::source_location::current();
	loguru::log((loguru::NamedVerbosity)verbosity, location.file_name(), location.line(), message.c_str());
}

/**
* @brief Make a basic assertion.
* @param expression
* @param message A text that will be logged in case of assertion failure.
*/
export inline void Assert(bool const expression, std::string const& message)
{
	CHECK_F(expression, message.c_str());
}

/**
 * @brief Checks if a provided pointer is not null
 * @tparam T1
 * @param ptr
 * @param message A custom message about the assertion failure, if left empty a default message will be logged.
*/
export template <typename T1>
inline void AssertNotNull(T1 const* ptr, std::string const& message = "")
{
	if (message.empty())
		CHECK_NOTNULL_F(ptr);
	else
		CHECK_NOTNULL_F(ptr, message.c_str());
}

/**
 * @brief Perform an assertion of a == b.
 * @tparam T1
 * @tparam T2
 * @param a
 * @param b
 * @param message A custom message about the assertion failure, if left empty a default message will be logged.
*/
export template <typename T1, typename T2 >
inline void AssertEqual(T1 const& a, T2 const& b, std::string const& message = "")
{
	if (message.empty())
		CHECK_OP_F(a, b, == );
	else
		CHECK_OP_F(a, b, == , message.c_str());
}

/**
 * @brief Perform an assertion of a != b.
 * @tparam T1
 * @tparam T2
 * @param a
 * @param b
 * @param message A custom message about the assertion failure, if left empty a default message will be logged.
*/
export template <typename T1, typename T2 >
inline void AssertNotEqual(T1 const& a, T2 const& b, std::string const& message = "")
{
	if (message.empty())
		CHECK_OP_F(a, b, != );
	else
		CHECK_OP_F(a, b, != , message.c_str());
}

/**
 * @brief Perform an assertion of a > b.
 * @tparam T1
 * @tparam T2
 * @param a
 * @param b
 * @param message A custom message about the assertion failure, if left empty a default message will be logged.
*/
export template <typename T1, typename T2 >
inline void AssertGreater(T1 const& a, T2 const& b, std::string const& message = "")
{
	if (message.empty())
		CHECK_OP_F(a, b, > );
	else
		CHECK_OP_F(a, b, > , message.c_str());
}

/**
 * @brief Perform an assertion of a < b.
 * @tparam T1
 * @tparam T2
 * @param a
 * @param b
 * @param message A custom message about the assertion failure, if left empty a default message will be logged.
*/
export template <typename T1, typename T2 >
inline void AssertLess(T1 const& a, T2 const& b, std::string const& message = "")
{
	if (message.empty())
		CHECK_OP_F(a, b, < );
	else
		CHECK_OP_F(a, b, < , message.c_str());
}

/**
 * @brief Perform an assertion of a >= b.
 * @tparam T1
 * @tparam T2
 * @param a
 * @param b
 * @param message A custom message about the assertion failure, if left empty a default message will be logged.
*/
export template <typename T1, typename T2 >
inline void AssertGreaterEqual(T1 const& a, T2 const& b, std::string const& message = "")
{
	if (message.empty())
		CHECK_OP_F(a, b, >= );
	else
		CHECK_OP_F(a, b, >= , message.c_str());
}

/**
 * @brief Perform an assertion of a <= b.
 * @tparam T1
 * @tparam T2
 * @param a
 * @param b
 * @param message A custom message about the assertion failure, if left empty a default message will be logged.
*/
export template <typename T1, typename T2 >
inline void AssertLessEqual(T1 const& a, T2 const& b, std::string const& message = "")
{
	if (message.empty())
		CHECK_OP_F(a, b, <= );
	else
		CHECK_OP_F(a, b, <= , message.c_str());
}

/**
* @brief Abort the program and log a message
* @param abortMessage
*/
export inline void Abort(std::string const& abortMessage)
{
	ABORT_F(abortMessage.c_str());
}

//TODO: Might be beneficial to include Error Context macro wrapper here

//TODO: Implement a Fatal_handler for cleanup!