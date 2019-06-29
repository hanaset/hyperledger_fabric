#define BOOST_LOG_ALL_LINK
#define BOOST_PHOENIX_NO_VARIADIC_FUNCTION_EVAL
#define BOOST_NO_CXX11_VARIADIC_TEMPLATES

#include "log.h"

#include <stdexcept>
#include <string>
#include <iostream>

#include <boost/core/null_deleter.hpp>
#include <boost/locale/generator.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/exception_handler_feature.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/exception_handler.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks/sync_frontend.hpp>

namespace src = boost::log::sources;
namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

Mlog::Mlog() {

    GlobalAttribute();
}

Mlog::~Mlog() {

    logging::core::get()->flush();
    logging::core::get()->remove_all_sinks();

}

void Mlog::GlobalAttribute() {

    logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
    logging::core::get()->add_global_attribute("RecordID", attrs::counter< unsigned int >());

}

void Mlog::AddTextFile() {

    boost::shared_ptr< logging::core > core = logging::core::get();

    boost::shared_ptr< sinks::text_file_backend > backend =
        boost::make_shared< sinks::text_file_backend >(
            keywords::file_name = "file_%3N.log",                                          /*< target file name pattern >*/
            //keywords:: = "file_%5N.log",
            keywords::rotation_size = 5 * 1024 * 1024,                                     /*< rotate the file upon reaching 5 MiB size... >*/
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0)  /*< ...or every day, at noon, whichever comes first >*/
            //keywords::channel = "file"
            );
    backend->auto_flush(true);

    typedef sinks::synchronous_sink< sinks::text_file_backend > sink_t;
    boost::shared_ptr< sink_t > sink(new sink_t(backend));

    sink->set_formatter(
        expr::format("[%1%] [%2%] [%3%] %4%")
        % expr::attr< unsigned int >("RecordID")
        % expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
        % logging::trivial::severity
        % expr::message
    );

    core->add_sink(sink);
}

void Mlog::AddConsole() {

    boost::shared_ptr<sinks::text_ostream_backend> backend = boost::make_shared<sinks::text_ostream_backend>();
    backend->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
    backend->auto_flush(true);

    typedef sinks::synchronous_sink<sinks::text_ostream_backend> sink_t;
    boost::shared_ptr<sink_t> sink(new sink_t(backend));


    sink->set_formatter(
        expr::format("[%1%] [%2%] [%3%] %4%")
        % expr::attr< unsigned int >("RecordID")
        % expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
        % logging::trivial::severity
        % expr::message
    );

    logging::core::get()->add_sink(sink);
}

