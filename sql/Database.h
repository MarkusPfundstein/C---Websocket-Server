#ifndef __SERVER_SQLMANAGER_HPP__
#define __SERVER_SQLMANAGER_HPP__

#include <string>
#include <mysql/mysql.h>
#include <map>
#include <vector>

namespace sql {
	
	typedef std::map<std::string, std::string> Dictionary;
	typedef std::vector<Dictionary> Collection;

	class Database
	{
		public:

			explicit Database();
			~Database();
			bool Startup(const std::string &username, const std::string& password, const std::string& database, const std::string& host);
			bool Startup(const std::string& username, const std::string& password, const std::string& database);
			
			inline const std::string& Name() const
			{
				return name_;
			}

			Collection Query(const std::string& predicate);

		private:
			MYSQL *sql_;	
			std::string name_;

			Database(const Database& rhs) = delete;
			Database(Database&& rhs) = delete;
			Database& operator=(const Database& rhs) = delete;
			Database& operator=(Database&& rhs) = delete;
	};

}

#endif
