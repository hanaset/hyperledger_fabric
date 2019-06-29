#define BOOST_LOG_DYN_LINK
#define BOOST_PHOENIX_NO_VARIADIC_FUNCTION_EVAL
#define BOOST_NO_CXX11_VARIADIC_TEMPLATES

#include <boost/core/null_deleter.hpp>
#include <boost/locale/generator.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/exception_handler_feature.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/exception_handler.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>


BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(slg, boost::log::sources::severity_logger< boost::log::trivial::severity_level >)

#define MLOG(lvl) BOOST_LOG_SEV(slg::get(), lvl)	\
	<< boost::log::add_value("File", __FILE__)		\
	<< boost::log::add_value("Line",__LINE__)		\
	<< boost::log::add_value("Function", __FUNCTION__)

class Mlog
{
public:
    Mlog();
    ~Mlog();
    void AddTextFile();
    void AddConsole();
private:
    void GlobalAttribute();
};
