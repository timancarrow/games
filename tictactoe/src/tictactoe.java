import java.util.Scanner;

public class tictactoe {
	
	public static class Gameboard{
		
		private int[][] board;
		private int xmax = 0, ymax = 0;
		
		//Create the gameboard. Specify the max X dimension, max Y dimension, and number of posssible states for each square.
		public Gameboard(int maxX, int maxY){
			
			xmax = maxX;
			ymax = maxY;
			board = new int[maxX][maxY];
		}
				
		/* Get state of square at (xdim,ydim). 
		 * Return -1 if dimensions are out of bounds.
		 */
		public int getState(int xdim, int ydim){
			
			if(xdim <= xmax && ydim <= ymax && xdim >= 0 && ydim >= 0){
				int state = board[xdim][ydim];
				return state;
			}
			else
				return -1;
		}
		
		/* Set state value for the square at (xdim,ydim).
		 * Return 0 for no error.
		 * Return -1 if dimensions are out of bounds.
		 */
		public int setState(int xdim, int ydim, int setvalue){
					
					if(xdim <= xmax && ydim <= ymax && xdim >= 0 && ydim >= 0){
							board[xdim][ydim] = setvalue;
							return 0;
						}
						else
							return -1;
				}

		//Clear the gameboard by setting it back to default values.
		public void clearBoard(){	 
			board = new int[xmax][ymax];
		}
		
		
		}
		
	
	//Generic print for a gameboard
	public static void printboard(int xmax, int ymax, Gameboard gameboard){
		
		int curstate = 0; 
		System.out.format("%n");
		for(int y = 0; y < ymax; y++){	
			
			System.out.format("|");
			for(int x = 0; x < xmax; x++){
			    curstate = gameboard.getState(x,y);
				System.out.format(" %d ", curstate);	
			}
			System.out.format("|%n");
		}
		System.out.println("");
	}
	
	//Check if there is a tictactoe winner. The return value of -1 or 1 indicates the winner. Return of 0 means no winner.
	public static int tttcheckWinner(Gameboard gameboard){
		int TLBRsum = 0, BLTRsum = 0, Hsum = 0, Vsum = 0;
		
		//Check TopLeft to BottomRight Diag
		for(int x = 0; x < gameboard.xmax; x++){
			TLBRsum = TLBRsum + gameboard.board[x][x];
		}	
		if(TLBRsum == 3){
			return 1;
		}
		
		if(TLBRsum == -3){
			return -1;
		}		
		
		//Check BottomLeft to TopRight Diag
		for(int x = 0; x < gameboard.xmax; x++){
			BLTRsum = BLTRsum + gameboard.board[x][gameboard.ymax-x-1];
		}	
		if(BLTRsum == 3){
			return 1;
		}
		
		if(BLTRsum == -3){
			return -1;
		}
		
		//Check Horizontals
		for(int y = 0; y < gameboard.ymax; y++){
			Hsum = 0; 
			for(int x = 0; x < gameboard.xmax; x++){
				Hsum = Hsum + gameboard.board[x][y];
				
				if(Hsum == 3){
					return 1;
				}
				
				if(Hsum == -3){
					return -1;
				}
			}
		}
		
		//Check Verticals
		for(int x = 0; x < gameboard.xmax; x++){
			Vsum = 0; 
			for(int y = 0; y < gameboard.ymax; y++){
				Vsum = Vsum + gameboard.board[x][y];
				
				if(Vsum == 3){
					return 1;
				}
				
				if(Vsum == -3){
					return -1;
				}
				
			}
		}
		
		return 0;
		
		
	}
	
	public static void main(String[] args) throws InterruptedException {
		
		Scanner reader = new Scanner(System.in);
		
		int xMax = 3, yMax = 3, winner = 0, turnNum = 1;
		boolean playerselect = true;
		
		Gameboard tttgameboard = new Gameboard(xMax, yMax);
		
		boolean stopGame = false;
		System.out.println("tictactest\n");
		
		int curX = 0, curY = 0;
		
		while(!stopGame){
			
			printboard(xMax, yMax, tttgameboard);
			
			if(playerselect){
				System.out.println("Player 1 choose a location 'x':");
				curX = reader.nextInt();
				
				System.out.println("Player 1 choose a location 'y':");
				curY = reader.nextInt();		
				if(tttgameboard.setState(curX, curY, 1 ) < 0)
				{
					System.out.println("Out of Bounds! Try again.");
				}
				else
					playerselect = !playerselect;
			
			}
			else{
				System.out.println("Player 2 choose a location 'x':");
				curX = reader.nextInt();
				
				System.out.println("Player 2 choose a location 'y':");
				curY = reader.nextInt();
				
				if(tttgameboard.setState(curX, curY, -1 ) < 0)
				{
					System.out.println("Out of Bounds! Try again.");
				}
				else
					playerselect = !playerselect;
			}
	
			//check winner
			winner = tttcheckWinner(tttgameboard);
			if(winner != 0){
				stopGame = true;
				
			//increment turn number
			turnNum = turnNum+1;
			
			if(turnNum == 9)
			{
				System.out.println("No winner for this game!");
			}
			
			}
		}
		
		System.out.format("The Winner is: %d%n", winner);
		printboard(xMax, yMax, tttgameboard);
		tttgameboard.clearBoard();
		reader.close();
	}
}