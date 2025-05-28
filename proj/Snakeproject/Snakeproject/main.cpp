#include <iostream>

#include "GameMenu.h"
#include "GameServer.h"
#include "GameClient.h"

#include <windows.h>

int main() {
    
	// Ukrycie konsoli 
    
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE);
    

    // Uruchomienie menu wyboru trybu gry
    while (true) {
        GameMenu menu;
        GameMode selectedMode = menu.run();

        sf::RenderWindow& menuWindow = menu.getWindow();

        // Sprawdzenie wybranego trybu SERVER / CLIENT
        if (selectedMode == GameMode::HOST) {
            std::cout << "Running as host..." << std::endl;
            GameServer server(menuWindow);
            server.run(menu.nicknameInput, menu.selectedColor);
        }
        else if (selectedMode == GameMode::CLIENT) {
            std::cout << "Running as client..." << std::endl;
            // Utworzenie i uruchomienie klienta
            GameClient client(menuWindow);
			// okno dialogowe do wpisania adresu IP
            if (client.showConnectDialog()) {
                client.run(menu.nicknameInput, menu.selectedColor);
            }
        }
        else break;
    }
    return 0;
}