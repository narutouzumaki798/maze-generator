// maze tileset --------------------------


// basic
void wall(int r, int c)
{
	mvaddwstr(r, c, L"\u2502");
}
void floor(int r, int c)
{
	mvaddwstr(r, c, L"\u2500");
}


// corners
void corner(int r, int c, int type)
{
	if( type == 0 || type == 1 || type == 2 ||
		type == 4 || type == 8) return;
	const wchar_t* ch[16];
	ch[ 5] = L"\u250c"; // top left corner                    8
	ch[ 6] = L"\u2510"; // top right corner                   |
	ch[ 9] = L"\u2514"; // bottom left corner                 |
	ch[10] = L"\u2518"; // bottom right corner         2 -----+------ 1
	                    //                                    |
	ch[11] = L"\u2534"; // t up                               |
	ch[ 7] = L"\u252c"; // t down                             4
	ch[14] = L"\u2524"; // t left
	ch[13] = L"\u251c"; // t rigt

	ch[15] = L"\u253c"; // plus

	ch[ 3] = L"\u2500"; // floor
	ch[12] = L"\u2502"; // wall

	if( type != 5  &&
		type != 6  && 
		type != 9  && 
		type != 10 && 
		type != 11 && 
		type != 7  && 
		type != 14 && 
		type != 13 &&
		type != 3  &&
		type != 12 &&
		type != 15 )
	{
		cerr << "Wrong Corner Type: " << type << endl; exit(1);
	}

	mvaddwstr(r, c, ch[type]);
}


















