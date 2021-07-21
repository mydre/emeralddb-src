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

#include "pmdOptions.hpp"
#include "pd.hpp"

pmdOptions::pmdOptions ()
{
   memset ( _dbPath, 0, sizeof(_dbPath) ) ;
   memset ( _logPath, 0, sizeof(_logPath) ) ;
   memset ( _confPath, 0, sizeof(_confPath) ) ;
   memset ( _svcName, 0, sizeof(_svcName) ) ;
   _maxPool = NUMPOOL ;
}

pmdOptions::~pmdOptions ()
{
}

int pmdOptions::readCmd ( int argc, char **argv,po::options_description &desc,po::variables_map &vm )
{
   int rc = EDB_OK ;
   try
   {
      po::store ( po::command_line_parser ( argc, argv ).options (desc ).allow_unregistered().run(), vm ) ;
      po::notify ( vm ) ;
   }
   catch ( po::unknown_option &e )
   {
      std::cerr << "Unknown arguments: "
                << e.get_option_name() << std::endl ;
      rc = EDB_INVALIDARG ;
      goto error ;
   }
   catch ( po::invalid_option_value &e )
   {
      std::cerr << "Invalid arguments: "
                << e.get_option_name() << std::endl ;
      rc = EDB_INVALIDARG ;
      goto error ;
   }
   catch ( po::error &e )
   {
      std::cerr << "Error: " << e.what() << std::endl ;
      rc = EDB_INVALIDARG ;
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}

int pmdOptions::importVM ( const po::variables_map &vm, bool isDefault )
{
   int rc = EDB_OK ;
   const char *p = NULL ;
   // conf path
   if ( vm.count ( PMD_OPTION_CONFPATH ) )
   {
      p = vm[PMD_OPTION_CONFPATH].as<string>().c_str() ;
      strncpy ( _confPath, p, OSS_MAX_PATHSIZE ) ;
   }
   else if ( isDefault )
   {
      strcpy ( _confPath, "./" CONFFILENAME ) ;

   }
   // log path
   if ( vm.count ( PMD_OPTION_LOGPATH ) )
   {
      p = vm[PMD_OPTION_LOGPATH].as<string>().c_str() ;
      strncpy ( _logPath, p, OSS_MAX_PATHSIZE ) ;
   }
   else if ( isDefault )
   {
      strcpy ( _logPath, "./" LOGFILENAME ) ;
   }
   // db file path
   if ( vm.count ( PMD_OPTION_DBPATH ) )
   {
      p = vm[PMD_OPTION_DBPATH].as<string>().c_str() ;
      strncpy ( _dbPath, p, OSS_MAX_PATHSIZE ) ;
      //acat@acat-xx:src$ ./emeralddb --dbpath fda.cc
      //acat@acat-xx:src$ ./emeralddb -d fda.cc
   }
   else if ( isDefault )
   {
      strcpy ( _dbPath, "./" DBFILENAME ) ;
   }

   // svcname
   if ( vm.count ( PMD_OPTION_SVCNAME ) )
   {
      p = vm[PMD_OPTION_SVCNAME].as<string>().c_str() ;
      strncpy ( _svcName, p, NI_MAXSERV ) ;
   }
   else if ( isDefault )
   {
      strcpy ( _svcName, SVCNAME ) ;
   }

   // maxpool(线程池大小)
   if ( vm.count ( PMD_OPTION_MAXPOOL ) )
   {
      _maxPool = vm [ PMD_OPTION_MAXPOOL ].as<unsigned int> () ;
   }
   else if ( isDefault )
   {
      _maxPool = NUMPOOL ;
   }
   return rc ;
}

int pmdOptions::readConfigureFile ( const char *path,
                                    po::options_description &desc,
                                    po::variables_map &vm )
{
   int rc                        = EDB_OK ;
   char conf[OSS_MAX_PATHSIZE+1] = {0} ;
   strncpy ( conf, path, OSS_MAX_PATHSIZE ) ;//后面使用conf读取配置
   try
   {
      po::store ( po::parse_config_file<char> ( conf, desc, true ), vm ) ;
      po::notify ( vm ) ;
   }
   catch( po::reading_file &e)
   {
      std::cerr << "Failed to open config file: "
                <<( std::string ) conf << std::endl
                << "Using default settings" << std::endl ;
      rc = EDB_IO ;
      goto error ;
   }
   catch ( po::unknown_option &e )
   {
      std::cerr << "Unkown config element: "
                << e.get_option_name () << std::endl ;
      rc = EDB_INVALIDARG ;
      goto error ;
   }
   catch ( po::invalid_option_value &e )
   {
      std::cerr << ( std::string ) "Invalid config element: "
                << e.get_option_name () << std::endl ;
      rc = EDB_INVALIDARG ;
      goto error ;
   }
   catch( po::error &e )
   {
      std::cerr << e.what () << std::endl ;
      rc = EDB_INVALIDARG ;
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}

int pmdOptions::init ( int argc, char **argv )
{
   int rc = EDB_OK ;
   po::options_description all ( "Command options" ) ;
   po::variables_map vm ;//命令行
   po::variables_map vm2 ;//文件

   //all.add_descptin()(str,str)(str,str,str)(str,str,str)(str,str,str);
   PMD_ADD_PARAM_OPTIONS_BEGIN( all )
      PMD_COMMANDS_OPTIONS
   PMD_ADD_PARAM_OPTIONS_END
   rc = readCmd ( argc, argv, all, vm ) ;//读取命令行,到vm当中
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to read cmd, rc = %d", rc ) ;
      goto error ;
   }
   // check if we have help options
   if ( vm.count ( PMD_OPTION_HELP ) )
   {
	  /*
	   * acat@acat-xx:src$ ./emeralddb -help
Command options:
  -h [ --help ]         help
  -d [ --dbpath ] arg   database file full path
  -s [ --svcname ] arg  local service name
  -m [ --maxpool ] arg  max pooled agent
  -l [ --logpath ] arg  diagnostic log file full path
  -c [ --confpath ] arg configuration file full path
	   * */
      std::cout << all << std::endl ;
      rc = EDB_PMD_HELP_ONLY ;
      goto done ;
   }else{
   }
   // check if there's conf path
   if ( vm.count ( PMD_OPTION_CONFPATH ) )
   {
      rc = readConfigureFile ( vm[PMD_OPTION_CONFPATH].as<string>().c_str(),all, vm2 ) ;
   }
   if ( rc )
   {
      PD_LOG ( PDERROR, "Unexpected error when reading conf file, rc = %d",
               rc ) ;
      goto error ;
   }
   // load vm from file
   rc = importVM ( vm2 ) ;//
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to import from vm2, rc = %d", rc ) ;
      goto error ;
   }
   // load vm from command line，说明命令行的优先权更高
   rc = importVM ( vm ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to import from vm, rc = %d", rc ) ;
      goto error ;
   }
done :
   return rc ;
error :
   goto done ;
}
