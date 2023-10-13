class GameBotPostResponse {

    struct Player {
    String name;
    int rating;
    int ratingDiff;
    String id;
    };

    struct Opening {
        String eco;
        String name;
        int ply;
    };

    struct Clock {
        int initial;
        int increment;
        int totalTime;
    };

    struct ChessGame {
        String id;
        bool rated;
        String variant;
        String speed;
        String perf;
        long long createdAt;
        long long lastMoveAt;
        String status;
        Player white;
        Player black;
        Opening opening;
        String moves;
        Clock clock;
    };

    private:
        ChessGame chessGame;
        Clock clock;
        Opening opening;
        Player player;
    
    public: 
    ChessGame getChessGame() {
        return this->chessGame;
    }

};