#include <bits/stdc++.h>
#include <unistd.h>
#include <ncurses.h>
#include <locale.h>


using namespace std;

#include "tileset.cpp"


class edge{
public:
  int i;
  int j;
  int dir; // 0=up, 1=down, 2=left, 3=right

  edge()
  {
    i = 0; j = 0; dir = 0;
  }

  edge(int a, int b, int c)
  {
    i = a; j = b; dir = c;
  }

  edge(const edge& e)
  {
    this->i = e.i;
    this->j = e.j;
    this->dir = e.dir;
  }

  void operator=(const edge& e)
  {
    this->i = e.i;
    this->j = e.j;
    this->dir = e.dir;
  }

  string direction()
  {
    if(dir == 0) return "up";
    if(dir == 1) return "down";
    if(dir == 2) return "left";
    if(dir == 3) return "right";
    return "ttt";
  }
};


class maze{
public:
  int width;
  int height;
  int corner_r;
  int corner_c;

  int up[100][100];        // adjacency flags for each cell
  int down[100][100];
  int left[100][100];
  int right[100][100];

  edge edge_list[40000];
  int num_edges;

  int parent[10000];        // parent array for union1 find
  int rank[10000];          // rank array for union1 find
  bool visited[100][100];

  int cell_id(int i, int j) // cell id from row and column
  {
    return (width*(i-1) + j);
  }
  int cell_row(int id)
  { return (id%width == 0)?(id/width):(id/width+1); }
  int cell_col(int id)
  { return (id%width == 0)?(width):(id%width);      }

  bool inside(int i, int j) // check if cell is inside maze
  {
    if(i < 1) return 0;
    if(j < 1) return 0;
    if(i > height) return 0;
    if(j > width) return 0;
    return 1;
  }

  void reset()
  {
    // initialize adjacency flags
    for(int i = 0; i <= height+1; i++)
      {
	for(int j = 0; j <= width+1; j++)
	  {
	    up[i][j] = 0;
	    down[i][j] = 0;
	    left[i][j] = 0;
	    right[i][j] = 0;

	    if(i == 0)        { up[i][j] = 1;    left[i][j] = 1; right[i][j] = 1; }
	    if(i == height+1) { down[i][j] = 1;  left[i][j] = 1; right[i][j] = 1; }
	    if(j == 0)        { left[i][j] = 1;  up[i][j] = 1;   down[i][j] = 1;  } 
	    if(j == width+1)  { right[i][j] = 1; up[i][j] = 1;   down[i][j] = 1;  } 
	  }
      }

    // initialize parent array for union1-find (all cells in separate sets with themselves as roots)
    // and visited array for dfs
    for(int i = 1; i <= height; i++)
      {
	for(int j = 1; j <= width; j++)
	  {
	    parent[cell_id(i, j)] = cell_id(i, j);
	    rank[cell_id(i, j)] = 0;
	    visited[i][j] = false;
	  }
      }
	  erase();
  }

  maze(int h, int w, int cr, int cc) // constructor
  {
    height = h; width = w;
    corner_r = cr; corner_c = cc;
    reset();
  }



  void generate_edge_list() // generate list of candidate edges
  {
    int idx = 0;
    for(int i=1; i<=height; i++)
      {
	for(int j=1; j<=width; j++)
	  {
	    if(i > 1) edge_list[idx++] = edge(i, j, 0);
	    if(i < height) edge_list[idx++] = edge(i, j, 1);
	    if(j > 1) edge_list[idx++] =  edge(i, j, 2);
	    if(j < width) edge_list[idx++] = edge(i, j, 3);
	  }
      }
    num_edges = idx;
  }

  void swap_edge(int i, int j) // swap edge i and edge j
  {
    edge tmp = edge_list[j];
    edge_list[j] = edge_list[i];
    edge_list[i] = tmp;
  }

  // up down left right
  int get_type(int i, int j) // find type of top-left corner for cell (r, c)
  {
    int ans = 0;
    if(down[i-1][j] == 0)  ans |= 1;
    if(down[i-1][j-1] == 0) ans |= 2;
    if(right[i][j-1] == 0) ans |= 4;
    if(right[i-1][j-1] == 0) ans |= 8;
    return ans;
  }


  void randomize() // generate a random permutation of the edge list
  {
    edge tmp_list[num_edges];
    int n = num_edges;
    int idx = 0;

    while(n > 0) // Fisher-Yates shuffle
      {
	int x = ((rand()%n)*(rand()%n))%n; x = (x + n)%n; // choose random number in [0...n-1]
	tmp_list[idx++] = edge_list[x];
	swap_edge(x, n-1);
	n--;
      }

    for(int i=0; i<num_edges; i++)
      edge_list[i] = tmp_list[i];
  }

  int find(int x) // find parent of cell with id x 
  {
    int p = x;
    if(parent[x] != x)
      p = find(parent[x]);
    parent[x] = p;
    return p;
  }

  void union1(int a, int b) // merge sets of cells with id a and b
  {
    int aroot = find(a);
    int broot = find(b);

    if(aroot == broot) return;

    if(rank[aroot] > rank[broot])
      {
	parent[broot] = aroot;
      }
    else if(rank[aroot] < rank[broot])
      {
	parent[aroot] = broot;
      }
    else // rank[aroot] == rank[broot]
      {
	parent[broot] = aroot;
	rank[aroot]++;
      }
  }




  void generate_kruskal()
  {
    mvprintw(corner_r+height*2 + 1, corner_c + 2, "Kruskal's Algorithm");
    generate_edge_list();
    randomize();

    int cyc_count = 0; // cycle count

    // kruskal's algorithm
    for(int i=0; i<num_edges; i++)
      {
	int a_row, a_col, b_row, b_col;
	edge e = edge_list[i];

	// cerr << "edge: " << e.i << ", " << e.j << " -- " << e.direction() << endl;

	a_row = e.i; a_col = e.j;     // row and column of 1st cell a
	b_row = a_row; b_col = a_col; // take the same row and column for 2nd cell b

	// update based on edge direction
	if(e.dir == 0) b_row--;       // up
	else if(e.dir == 1) b_row++;  // down
	else if(e.dir == 2) b_col--;  // left
	else if(e.dir == 3) b_col++;  // right
	else // error
	  {
	    printf("dir error 1\n");
	    exit(1);
	  }

	int a = cell_id(a_row, a_col); // cell id of 1st cell a
	int b = cell_id(b_row, b_col); // cell id of 2nd cell b

	if(find(a) == find(b))         // the cells belong to the same set
	  {                              // i.e, adding this edge will form cycle
	    if(cyc_count == 10) continue;
	    cyc_count++;
	  }

	union1(a, b); // merge sets containg cells a and b

	// update adjacency flag
	if(e.dir == 0) // up
	  {
	    up[a_row][a_col] = 1;
	    down[b_row][b_col] = 1;
	  }
	else if(e.dir == 1) // down
	  {
	    down[a_row][a_col] = 1;
	    up[b_row][b_col] = 1;
	  }
	else if(e.dir == 2) // left
	  {
	    left[a_row][a_col] = 1;
	    right[b_row][b_col] = 1;
	  }
	else if(e.dir == 3) // right
	  {
	    right[a_row][a_col] = 1;
	    left[b_row][b_col] = 1;
	  }
	else // error
	  {
	    printf("dir error 2\n");
	    exit(1);
	  }

	// show();
	// usleep(100000);
      }
  }

  void dfs(int r, int c)
  {
    visited[r][c] = true;
    while(1)
      {
	vector<int> dir_available; // list of available directions 
	int r1, c1;

	r1 = r-1, c1 = c;
	if(inside(r1, c1) && !visited[r1][c1]) dir_available.push_back(1); // up
	r1 = r+1, c1 = c;
	if(inside(r1, c1) && !visited[r1][c1]) dir_available.push_back(2); // down
	r1 = r, c1 = c-1;
	if(inside(r1, c1) && !visited[r1][c1]) dir_available.push_back(3); // left
	r1 = r, c1 = c+1;
	if(inside(r1, c1) && !visited[r1][c1]) dir_available.push_back(4); // right

	if(dir_available.size() == 0) break; // nowhere to go

	int go = dir_available[rand()%((int)dir_available.size())];
	if(go == 1) // up
	  {
	    up[r][c] = 1; down[r-1][c] = 1;
	    dfs(r-1, c);
	  }
	else if(go == 2) // down
	  {
	    down[r][c] = 1; up[r+1][c] = 1;
	    dfs(r+1, c);
	  }
	else if(go == 3) // left
	  {
	    left[r][c] = 1; right[r][c-1] = 1;
	    dfs(r, c-1);
	  }
	else if(go == 4) // right
	  {
	    right[r][c] = 1; left[r][c+1] = 1;
	    dfs(r, c+1);
	  }
	else
	  {
	    printf("dfs error 1\n\n"); exit(1);
	  }

      }
 }

  void generate_recursive_backtracker()
  {
    mvprintw(corner_r+height*2 + 1, corner_c + 2, "Recursive Backtracker");
    dfs(1, 1);
  }


  void traverse()
  {
    int pre[100][100]; // etai visited[i][j] for traversal
    int next[40000];
    for(int i=1; i<=height; i++)
      for(int j=1; j<=width; j++)
	pre[i][j] = -1;

    deque<int> Q;
    Q.push_back(cell_id(1, 1)); pre[1][1] = 1;
    while(!Q.empty())
      {
	int x = Q.front(); Q.pop_front();
	int i = cell_row(x);
	int j = cell_col(x);

	cerr << i << ", " << j << " pre: " << cell_row(pre[i][j]) << ", " << cell_col(pre[i][j]) << endl;

	if(up[i][j] && pre[i-1][j] == -1)
	  {
	    Q.push_back(cell_id(i-1, j)); pre[i-1][j] = cell_id(i, j);
	  }
	if(down[i][j] && pre[i+1][j] == -1)
	  {
	    Q.push_back(cell_id(i+1, j)); pre[i+1][j] = cell_id(i, j);
	  }
	if(left[i][j] && pre[i][j-1] == -1)
	  {
	    Q.push_back(cell_id(i, j-1)); pre[i][j-1] = cell_id(i, j);
	  }
	if(right[i][j] && pre[i][j+1] == -1)
	  {
	    Q.push_back(cell_id(i, j+1)); pre[i][j+1] = cell_id(i, j);
	  }
      }

    cerr << "bfs hoe geche" << endl; fflush(stderr);

    int x = cell_id(height, width);
    while(x != cell_id(1, 1))
      {
	int i = cell_row(x);
	int j = cell_col(x);
	// cerr << "pre: " << i << ", " << j << endl; fflush(stderr);

	next[pre[i][j]] = x;
	x = pre[i][j];
      }


    x = cell_id(1, 1);
    while(1)
      {
	int i = cell_row(x);
	int j = cell_col(x);
	mvprintw(corner_r + 1 + (i-1)*2, corner_c + 2 + (j-1)*4, "x"); 
	if(x == cell_id(height, width)) break;
	x = next[x];
	usleep(100000);
	refresh();
      }

    cerr << "travell kora hoe geche" << endl; fflush(stderr);
    refresh();
  }

  void show()
  {
    int r = corner_r;
    int c = corner_c;
    for(int j = 1; j<=width+1; j++)
      {
	for(int i = 1; i<=height+1; i++)
	  {
	    // cerr << "show: cell " << i << ", " << j << endl; fflush(stderr);

	    int t;
	    // look north-west
	    t = get_type(i, j);
	    corner(r, c, t);
	    c++;

	    // north
	    if( down[i-1][j] == 0) floor(r, c); c++;
	    if( down[i-1][j] == 0) floor(r, c); c++;
	    if( down[i-1][j] == 0) floor(r, c);
	    c -= 3; r++;

	    // west
	    if(right[i][j-1] == 0) wall(r, c);
	    r++;
	  }
	r = corner_r; c += 4;
      }
    refresh();
  }

  void debug()
  {
    for(int i=1; i<=height; i++)
      {
	for(int j=1; j<=width; j++)
	  {
	    // cerr << "cell : " << i << ", " << j << ": ";
	    // if(up[i][j]) cerr << "up, ";
	    // if(down[i][j]) cerr << "down, ";
	    // if(left[i][j]) cerr << "left, ";
	    // if(right[i][j]) cerr << "right";
	    // cerr << endl;
	  }
      }
  }

};

maze init_maze()
{
  int width = 50; 
  int height = 25;
  int corner_r = 3;
  int corner_c = 5;// (COLS - (3*width+1))/2;
  maze a(height, width, corner_r, corner_c);
  return a;
}

void curses_init()
{
  initscr();
  noecho();
  curs_set(0);
  refresh();
  setlocale(LC_ALL, "");
  erase();
  refresh();
}
void curses_end()
{
  endwin();
}

int algorithm_selection()
{
  return 2;  // fixing
  erase();
  mvprintw(14, 10, "Enter Algorithm:");
  mvprintw(15, 10, "1. recursive backtracker");
  mvprintw(16, 10, "2. Kruskal's algorithm");
  mvprintw(17, 10, "3. exit");
  refresh();
  char ch = getch();
  erase();
  if(ch == '1') return 1;
  else if(ch == '2') return 2;
  else return 3;
}

int main()
{
  freopen("debug.txt", "w", stderr);
  srand(time(0));

  curses_init();
  maze a = init_maze();
  while(1)
    {
      a.reset();

      int ch = algorithm_selection();
      if(ch == 1) a.generate_recursive_backtracker();
      else if(ch == 2) a.generate_kruskal();
      else break;

      a.debug();
      a.show();
      a.traverse();
      usleep(500000);
      // getch();
    }
  // end
  getch();
  curses_end();
  return 0;
}

