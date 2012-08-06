#include <iostream>
#include <stdio.h>
#include <sstream>
#include "Database.h"

namespace sql {

	Database::Database()
		: sql_(NULL),
		  name_("")
	{
	
	}

	Database::~Database()
	{
		if (sql_) {
			mysql_close(sql_);
			sql_ = NULL;
			std::cout << "SQL database: \"" << name_ << "\" disconnected" << std::endl;
		}

		mysql_library_end();
	}

	Collection Database::Query(const std::string& predicate)
	{
		std::cout << "Do query" << std::endl;
		Collection query_result;
		if (mysql_query(sql_, predicate.c_str()) == 0) {
			std::cout << "Did query 1" << std::endl;
			MYSQL_RES *result = mysql_store_result(sql_);
			const int field_count = mysql_num_fields(result);

			std::vector<std::string> fields;

			while (MYSQL_FIELD* field = mysql_fetch_field(result)) {
				fields.push_back(field->name);
				std::cout << "field name" << field->name << std::endl;
			}

			while (MYSQL_ROW row = mysql_fetch_row(result)) {
				Dictionary new_dictionary;
				for (int i = 0; i < field_count; ++i) {
					const char* field_value = row[i];
					std::cout << "Field value: " << field_value << std::endl;
					if (field_value) {
						new_dictionary[fields[i]] = field_value;
					}
					else {
						new_dictionary[fields[i]] = "null";
					}
				}
				if (new_dictionary.size() > 0) {
					query_result.push_back(std::move(new_dictionary));
				}
			}
			mysql_free_result(result);
		}
		else {
			std::cerr << "Query error" << std::endl;
		}
		return std::move(query_result);
	}

	bool Database::Startup(const std::string &username, const std::string& password, const std::string& database, const std::string& host)
	{
		try {
			if (mysql_library_init(0, NULL, NULL)) {
				throw "mysql_library_init";
			}

			if ((sql_ = mysql_init(NULL)) == NULL) {
				throw "mysql_init";
			}

			if (mysql_real_connect(sql_, host.c_str(), username.c_str(), password.c_str(), database.c_str(), 0, NULL, 0) == NULL) {
				throw "mysql_real_connect";
			}

			name_ = database;
			
		} catch (char const* str) {
			std::cerr << "User defined exception: " << str << " - " << mysql_error(sql_) << std::endl;
			return false;
		}

		std::cout << "SQL database: \"" << name_ << "\" connected" << std::endl;

		return true;
	}
	
	bool Database::Startup(const std::string &username, const std::string& password, const std::string& database)
	{
		return Startup(username, password, database, "localhost");
	}

} // namespace sql
