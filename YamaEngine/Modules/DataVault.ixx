export module DataVault;
import <string>;
import <any>;
import <unordered_map>; // Unordered map as there are no expectations to go through the entire list, instead uses logic "you get only what you send"

// Using std::any because: 1) There's really no insurance that the data will be deleted, 2) overhead doesn't really matter as this won't be used as much - only for sending data between the scenes

/**
 * @brief Data holder meant to be used to store data that will survive the scene change.
*/
export std::unordered_map<std::string, std::any> DataVault;