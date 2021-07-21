/*********************************************************************************************
The GNU Affero General Public License is a free, copyleft license for software and other kinds of works,
specifically designed to ensure cooperation with the community in the case of network server software.
The licenses for most software and other practical works are designed to take away your freedom to share and
change the works. By contrast, our General Public Licenses are intended to guarantee your freedom to share
and change all versions of a program--to make sure it remains free software for all its users.
When we speak of free software, we are referring to freedom, not price. Our General Public Licenses are
designed to make sure that you have the freedom to distribute copies of free software
(and charge for them if you wish), that you receive source code or can get it if you want it,
that you can change the software or use pieces of it in new free programs, and that you know you can do
these things.Developers that use our General Public Licenses protect your rights with two steps:
(1) assert copyright on the software, and (2) offer you this License which gives you legal permission to copy,
distribute and/or modify the software.
**********************************************************************************************/

#ifndef PMDOPTIONS_HPP__
#define PMDOPTIONS_HPP__

#include "core.hpp"
#include<boost/program_options.hpp>
#include<boost/program_options/parsers.hpp>

using namespace std;
namespace po = boost::program_options;

#define PMD_OPTION_HELP                  "help"
#define PMD_OPTION_DBPATH                "dbpath"
#define PMD_OPTION_SVCNAME               "svcname"
#define PMD_OPTION_MAXPOOL               "maxpool"
#define PMD_OPTION_LOGPATH               "logpath"
#define PMD_OPTION_CONFPATH              "confpath"

#define PMD_ADD_PARAM_OPTIONS_BEGIN( desc )\
        desc.add_options()

#define PMD_ADD_PARAM_OPTIONS_END ;

#define PMD_COMMANDS_STRING(a,b) (string(a) + string(b)).c_str()

#define PMD_COMMANDS_OPTIONS \
        ( PMD_COMMANDS_STRING ( PMD_OPTION_HELP, ",h"), "help" ) \
        ( PMD_COMMANDS_STRING ( PMD_OPTION_DBPATH, ",d"), boost::program_options::value<string>(), "database file full path" ) \
        ( PMD_COMMANDS_STRING ( PMD_OPTION_SVCNAME, ",s"), boost::program_options::value<string>(), "local service name" ) \
        ( PMD_COMMANDS_STRING ( PMD_OPTION_MAXPOOL, ",m"), boost::program_options::value<unsigned int>(), "max pooled agent" ) \
        ( PMD_COMMANDS_STRING ( PMD_OPTION_LOGPATH, ",l"), boost::program_options::value<string>(), "diagnostic log file full path" ) \
        ( PMD_COMMANDS_STRING ( PMD_OPTION_CONFPATH, ",c"), boost::program_options::value<string>(), "configuration file full path" ) \

#define CONFFILENAME "edb.conf"
#define LOGFILENAME  "diag.log"
#define DBFILENAME   "edb.data"
#define SVCNAME      "48127"
#define NUMPOOL      5

class pmdOptions
{
public :
   pmdOptions () ;
   ~pmdOptions () ;
public :
   int readCmd ( int argc, char **argv,
                 po::options_description &desc,
                 po::variables_map &vm ) ;
   int importVM ( const po::variables_map &vm, bool isDefault = true ) ;
   int readConfigureFile ( const char *path,
                           po::options_description &desc,
                           po::variables_map &vm ) ;
   int init ( int argc, char **argv ) ;
public :
   // inline functions
   inline char *getDBPath ()
   {
      return _dbPath ;
   }
   inline char *getLogPath ()
   {
      return _logPath ;
   }
   inline char *getConfPath ()
   {
      return _confPath ;
   }
   inline char *getServiceName()
   {
      return _svcName ;
   }
   inline int getMaxPool ()
   {
      return _maxPool ;
   }
private :
   char _dbPath [ OSS_MAX_PATHSIZE+1 ] ;
   char _logPath [ OSS_MAX_PATHSIZE+1 ] ;
   char _confPath [ OSS_MAX_PATHSIZE+1 ] ;
   char _svcName [ NI_MAXSERV+1 ] ;
   int  _maxPool ;
} ;

#endif
