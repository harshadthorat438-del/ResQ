#include <iostream>
#include <string>
#include <limits>
#include <filesystem>
#include "rescue_queue.h"
#include "resources.h"
#include "locations.h"
#include "history.h"

namespace fs = std::filesystem;

void ensureDataDirectory() {
    try {
        if (!fs::exists("data")) {
            fs::create_directories("data");
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Could not create data directory: " << e.what() << "\n";
    }
}

std::string getCleanLine(const std::string& prompt) {
    std::string input;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, input)) {
            return "";
        }
        // Trim leading and trailing whitespace
        size_t first = input.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            std::cout << "Input cannot be empty. Please try again.\n";
            continue;
        }
        size_t last = input.find_last_not_of(" \t\r\n");
        std::string trimmed = input.substr(first, (last - first + 1));

        if (trimmed.find('|') != std::string::npos) {
            std::cout << "Character '|' is reserved. Please omit it and try again.\n";
            continue;
        }
        return trimmed;
    }
}

int getPositiveInt(const std::string& prompt) {
    int value = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value > 0) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cout << "Invalid input. Please enter a positive integer.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

int main() {
    ensureDataDirectory();

    RescueQueue rescueQueue;
    ResourceManager resourceManager;
    LocationGraph locationGraph;
    OperationHistory history;

    // Load persisted state from text files
    rescueQueue.loadFromFile();
    resourceManager.loadFromFile();
    locationGraph.loadFromFile();
    history.loadFromFile();

    int choice = -1;

    do {
        std::cout << "\n====================================\n";
        std::cout << " DISASTER RESCUE & RESOURCE SYSTEM\n";
        std::cout << "====================================\n";
        std::cout << "1. Add rescue request\n";
        std::cout << "2. View pending requests\n";
        std::cout << "3. Process next request\n";
        std::cout << "4. Add/replenish resource\n";
        std::cout << "5. View resource inventory\n";
        std::cout << "6. Allocate resource\n";
        std::cout << "7. Add location\n";
        std::cout << "8. Add road\n";
        std::cout << "9. BFS/DFS traversal\n";
        std::cout << "10. View operation history\n";
        std::cout << "0. Save and exit\n";
        std::cout << "Enter choice: ";

        if (!(std::cin >> choice)) {
            std::cout << "Please enter a valid number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: {
                std::string name = getCleanLine("Person/group name: ");
                std::string loc = getCleanLine("Affected location: ");
                std::string emergency = getCleanLine("Emergency type: ");
                int people = getPositiveInt("Number of people: ");

                int reqId = rescueQueue.enqueue(name, loc, emergency, people);
                if (reqId > 0) {
                    std::string msg = "Request #" + std::to_string(reqId) + " added (" + name + " at " + loc + ")";
                    history.push(msg);
                    std::cout << "Request #" << reqId << " added to the queue.\n";
                }
                break;
            }
            case 2: {
                rescueQueue.display();
                break;
            }
            case 3: {
                RescueRequest req(0, "", "", "", 0);
                if (rescueQueue.dequeue(req)) {
                    std::cout << "Processing request #" << req.id << " for " << req.name
                              << " at " << req.location << " (" << req.people << " people).\n";
                    std::string msg = "Processed request #" + std::to_string(req.id);
                    history.push(msg);
                    std::cout << "Request removed from pending queue.\n";
                } else {
                    std::cout << "No pending requests in the queue.\n";
                }
                break;
            }
            case 4: {
                std::string name = getCleanLine("Resource name: ");
                int qty = getPositiveInt("Quantity to add: ");

                if (resourceManager.addOrReplenish(name, qty)) {
                    std::string msg = "Stock updated: " + name + " (+" + std::to_string(qty) + ")";
                    history.push(msg);
                    std::cout << "Resource stock updated successfully.\n";
                }
                break;
            }
            case 5: {
                resourceManager.display();
                break;
            }
            case 6: {
                std::string name = getCleanLine("Resource to allocate: ");
                int qty = getPositiveInt("Quantity to allocate: ");

                int remaining = 0;
                if (resourceManager.allocate(name, qty, remaining)) {
                    std::string msg = "Allocated " + std::to_string(qty) + " " + name;
                    history.push(msg);
                    std::cout << "Allocation successful. Remaining " << name << ": " << remaining << "\n";
                }
                break;
            }
            case 7: {
                std::string name = getCleanLine("New location name: ");
                if (locationGraph.addLocation(name)) {
                    std::string msg = "Location added: " + name;
                    history.push(msg);
                    std::cout << "Location added successfully.\n";
                }
                break;
            }
            case 8: {
                std::string first = getCleanLine("First location: ");
                std::string second = getCleanLine("Second location: ");

                if (locationGraph.addRoad(first, second)) {
                    std::string msg = "Road added: " + first + " - " + second;
                    history.push(msg);
                    std::cout << "Road connected successfully.\n";
                }
                break;
            }
            case 9: {
                std::string start = getCleanLine("Starting location: ");
                std::cout << "1. BFS\n2. DFS\nChoice: ";
                int tChoice = 0;
                if (std::cin >> tChoice) {
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    if (tChoice == 1) {
                        locationGraph.bfs(start);
                    } else if (tChoice == 2) {
                        locationGraph.dfs(start);
                    } else {
                        std::cout << "Invalid traversal option.\n";
                    }
                } else {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid choice.\n";
                }
                break;
            }
            case 10: {
                history.display();
                break;
            }
            case 0: {
                rescueQueue.saveToFile();
                resourceManager.saveToFile();
                locationGraph.saveToFile();
                history.saveToFile();
                std::cout << "All data saved successfully. Goodbye!\n";
                break;
            }
            default:
                std::cout << "Invalid choice. Please select from the menu (0-10).\n";
                break;
        }
    } while (choice != 0);

    return 0;
}
