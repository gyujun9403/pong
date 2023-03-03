namespace pong
{
    public partial class PongBoard : Form
    {
    private
        // Location Variables
        int cpuDirection = 5;
        int ballXCoordinate = 5;
        int ballYCoordinate = 5;
        // Score Variables
        int playerScore = 0;
        int cpuScore = 0;
        // Size Variables
        int bottomBoundary;
        int centerPoint;
        int xMidpoint;
        int yMidpoint;
        // Detection Variables
        bool playerDetectedUp;
        bool playerDetectedDown;
        // Special Keys
        int spaceBarClicked = 0;
        public PongBoard()
        {
            InitializeComponent();
            bottomBoundary = (ClientSize.Height - 300) - player1.Height;
            xMidpoint = ClientSize.Width / 2;
            yMidpoint = (ClientSize.Height - 300) / 2;
        }

        private void PongBoard_Load(object sender, EventArgs e)
        {

        }

        // 특정 시간 간격으로 공이 hit했는지 검증
        private void pongTimer_Tick(object sender, EventArgs e)
        {
            Random newBallSpot = new Random();
            int newSpot = newBallSpot.Next(100, (ClientSize.Height- 300) - 100);
            // Adjust where the ball is
            pongBall.Top -= ballYCoordinate;
            pongBall.Left -= ballXCoordinate;
            // Make the CPU move
            cpuPlayer.Top += cpuDirection;
            // Check if CPU has reached the top or the bottom
            if (cpuPlayer.Top < 0 || cpuPlayer.Top > bottomBoundary) { cpuDirection = -cpuDirection; }
            // Check if the ball has exited the left side of the screen
            if (pongBall.Left < 0)
            {
                pongBall.Left = xMidpoint;
                pongBall.Top = newSpot;
                ballXCoordinate = -ballXCoordinate;
                cpuScore++;
                //cpuScoreLabel.Text = cpuScore.ToString();
            }

            // Check if the ball has exited the right side of the screen
            if (pongBall.Left + pongBall.Width > ClientSize.Width)
            {
                pongBall.Left = xMidpoint;
                pongBall.Top = newSpot;
                ballXCoordinate = -ballXCoordinate;
                playerScore++;
                //playerScoreLabel.Text = playerScore.ToString();
            }

            // Ensure the ball is within the boundaries of the screen
            if (pongBall.Top < 0 || pongBall.Top + pongBall.Height > (ClientSize.Height - 300)) { ballYCoordinate = -ballYCoordinate; }

            // Check if the ball hits the player or CPU paddle
            if (pongBall.Bounds.IntersectsWith(player1.Bounds) || pongBall.Bounds.IntersectsWith(cpuPlayer.Bounds))
            {
                // Send ball opposite direction
                ballXCoordinate = -ballXCoordinate;
            }

            // Move player up
            if (playerDetectedUp == true && player1.Top > 0) { player1.Top -= 10; }
            // Move player down
            if (playerDetectedDown == true && player1.Top < bottomBoundary) { player1.Top += 10; }
            // Check for winner
            if (playerScore >= 10) { pongTimer.Stop(); }
        }

        private void pongBall_Click(object sender, EventArgs e)
        {

        }

        private void PongBoard_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Up) { playerDetectedUp = false; }
            if (e.KeyCode == Keys.Down) { playerDetectedDown = false; }
        }

        private void PongBoard_KeyDown(object sender, KeyEventArgs e)
        {
            // If player presses the up arrow, move paddle upwards
            if (e.KeyCode == Keys.Up) { playerDetectedUp = true; }
            // If player presses the down arrow, move paddle downwards
            if (e.KeyCode == Keys.Down) { playerDetectedDown = true; }
            // If player presses the C key, open the character selection screen
            //if (e.KeyCode == Keys.C)
            //{
            //    Form character = new CharacterForm();
            //    character.Owner = this;
            //    pongTimer.Stop();
            //    character.Show();
            //}

            // If player presses space bar, pause the game
            if (e.KeyCode == Keys.Space)
            {
                if (spaceBarClicked % 2 == 0)
                {
                    pongTimer.Stop();
                }
                else
                {
                    pongTimer.Start();
                }
            }
            spaceBarClicked++;
        }

        private void splitter1_SplitterMoved(object sender, SplitterEventArgs e)
        {

        }

        private void groupBox1_Enter(object sender, EventArgs e)
        {

        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }
    }
}