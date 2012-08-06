#include "../Database.h"
#include <string>
#include <sstream>
#include <iostream>


sql::Database g_database;

static void print_cards()
{
	std::cout << "enter user id (numerical) or (type 'all' if you want to display all cards in database) " << std::endl;
	std::string user_id;
	std::cin >> user_id;
	std::ostringstream oss;
	if (user_id == "all") {
		oss << "SELECT * FROM cards;";
	} else {
		oss << "SELECT * FROM cards WHERE id = ANY (SELECT card_id FROM rel_user_cards WHERE user_id = '" << user_id << "');";
	}

	std::string predicate = oss.str();

	std::cout << predicate << std::endl;

	sql::Collection collection = g_database.Query(predicate);
	std::cout << " Query finished" << std::endl;
	auto it = collection.begin();
	for (; it != collection.end(); ++it) {
		sql::Dictionary dict = *it;
		auto dict_it = dict.begin();
		for (; dict_it != dict.end(); ++dict_it) {
			std::cout << dict_it->first << " - " << dict_it->second << std::endl;
		}
	}

#if 0
	sql::fetch_request<steampunk::card> fetch_request(predicate);
	
	sql::collection<steampunk::card> query_result = g_database.execute(fetch_request);
	sql::collection<steampunk::card>::const_iterator it = query_result.begin();
	
	oss.clear();

	std::ostringstream output;

	output << "your cards: \n";
	
	unsigned int counter = 0;
	for (; it != query_result.end(); ++it) {
		boost::shared_ptr<steampunk::card> card = *it;
		output << ++counter << " - " << card->card_name(); 
		if (counter < query_result.size()) {
			output << "\n";
		}
	}


	std::string complete = output.str();
	std::cout << complete << std::endl;
#endif
}

#if 0
static void insert_card()
{
	try {
		std::string new_card_name;
		std::cout << "enter name of the new card you want to insert: ";
		std::cin >> new_card_name;
		if (new_card_name.length() < 3) {
			throw "cardname < 3";
		}

		std::ostringstream oss;
		oss << "INSERT INTO cards (pk_card_id, card_name, script, cost, rarity) VALUES ('null', '";
		oss << new_card_name << "', 'test.scr', '0', '0')";

		std::string predicate(oss.str());

		sql::insert_request<steampunk::card> insert_request(predicate);

		boost::shared_ptr<steampunk::card> inserted_card = g_database.execute(insert_request);
		std::cout << "inserted card: [" << inserted_card->card_id() << "] " << inserted_card->card_name() << std::endl;

	} catch (char const* str) {
		std::cout << "exception: " << str << std::endl;
	}

}
#endif

int main(int argc, char **argv)
{
	if (!g_database.Startup(argv[1], argv[2], argv[3], argv[4])) {
		return -1;
	}
	
	int input = 0;
	do 
	{
		std::string input_buffer;
		std::cout << "(0) exit (1) print (2) insert" << std::endl;
		std::cin >> input_buffer;
		if (input_buffer == "0") {
			input = 0;
		}
		else if (input_buffer == "1") {
			print_cards();
		}
		else {
			input = 0;
		}
	} while (input);
	
	return 0;
}
