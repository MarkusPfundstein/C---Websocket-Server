#include <iostream>
#include <libjson/libjson.h>

int main(int argc, char **argv)
{
	
	JSONNODE *node = json_new(JSON_NODE);
	
	json_push_back(node, json_new_a(JSON_TEXT("name"), JSON_TEXT("markus")));
	json_push_back(node, json_new_i(JSON_TEXT("id"), 666));


	JSONNODE *cards = json_new(JSON_ARRAY);
	json_set_name(cards, "cards");

	const char* names[5] = { "Warrior", "Mage", "Leopard", "Potion", "Arrow" };
	int ids[5] = { 64, 32, 16, 99, 66 };

	for (int i = 0; i < 5; i++) {
		JSONNODE *card = json_new(JSON_NODE);

		json_push_back(card, json_new_a(JSON_TEXT("name"), names[i]));
		json_push_back(card, json_new_i(JSON_TEXT("id"), ids[i]));

		json_push_back(cards, card);

		//json_delete(card);
	}

	json_push_back(node, cards);

	json_char *text = json_write_formatted(node);
	std::cout << text << std::endl;
	json_free(text);

	json_delete(node);

	return 0;
}
