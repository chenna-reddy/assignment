#include <iostream>
#include <string>
#include <sstream>

/**
 * Player Choice
 */
enum class Choice {
    Paper,
    Scissor,
    Rock
};

std::ostream& operator<<(std::ostream& os, Choice c) {
    switch (c) {
        case Choice::Paper:
            os << "Paper";
            break;
        case Choice::Scissor:
            os << "Scissor";
            break;
        case Choice::Rock:
            os << "Rock";
            break;
        default:
            throw std::domain_error("Not all switch cases covered");
    }
    return os;
}

class Game {
public:
    /**
     * Play Method. Returns who is the winner
     * @param player1 Player 1 Choice
     * @param player2 Player 2 Choice
     * @return winner
     */
    int play(Choice player1, Choice player2) {
        if (player1 == player2) {
            // Draw
            return 0;
        } else if ((static_cast<int>(player2) + 1) % 3 == static_cast<int>(player1)) {
            // Player2 Choice is before Player1 Choice
            return 1;
        } else {
            return 2;
        }
    }
};


/**
 * Class to represent Player
 */
class Player {
    virtual Choice choice() = 0;
};

/**
 * Computer Player
 */
class Computer : public Player {
public:
    Choice choice() {
        return static_cast<Choice>(std::rand() % 3);
    }
};

/**
 * Human Player
 */
class Human : public Player {
public:
    Choice choice() {
        char ch;
        while(true) {
            std::cout << "r for Rock" << std::endl
                      << "p for Paper" << std::endl
                      << "s for Scissor" << std::endl
                      << "Next Move?" << std::endl;

            std::cin >> ch;
            switch (std::tolower(ch)) {
                case 'r':
                    return Choice::Rock;
                case 'p':
                    return Choice::Paper;
                case 's':
                    return Choice::Scissor;
                default:
                    std::cout << "Wrong choice " << ch << std::endl;
            }
        }
    }
};

/**
 * Controller for Playing Game
 */
class GameController {
public:
    /**
     * Game Controller for given Game
     * @param g Game
     */
    GameController(Game& g) : game(g), rounds(0), computerWon(0), humanWon(0) {}

    /**
     * Play one round
     */
    void play() {
        Choice computerChoice = computer.choice();
        Choice humanChoice = human.choice();
        int winner = game.play(computerChoice, humanChoice);
        switch (winner) {
            case 0:
            std::cout << "Its a draw!!" << std::endl;
                break;
            case 1:
                computerWon++;
            std::cout << computerChoice << " beats " << humanChoice << ". Better luck :-( !!" << std::endl;
                break;
            case 2:
                humanWon++;
                std::cout << humanChoice << " beats " << computerChoice << ". Well done :-) !!" << std::endl;
                break;
            default:
                throw std::domain_error("Invalid result from Game");
        }
        rounds++;
    }

    /**
     * Status of the games played till Now
     * @return Status as String
     */
    std::string status() const {
        std::ostringstream os;
        if (rounds == 0) {
            os << "No rounds Played";
        } else {
            os << "Out of " << rounds << " rounds " << " you won " << humanWon << ", compute won " << computerWon;
        }
        return os.str();
    }

private:
    Game& game;
    Computer computer;
    Human human;
    int rounds;
    int computerWon;
    int humanWon;
};


int main(int argc, char *argv[]) {
    int noOfGames;
    if (argc > 1) {
        noOfGames = std::stoi(argv[1]);
    } else {
        std::cout << "How many rounds do you want to play?" << std::endl;
        std::cin >> noOfGames;
    }
    Game game;
    GameController gameController(game);
    for (int i=0; i<noOfGames; ++i) {
        std::cout << "Round " << i+1 << "/" << noOfGames << std::endl;
        gameController.play();
    }
    std::cout << gameController.status() << std::endl;
    return 0;
}