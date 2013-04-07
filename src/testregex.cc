#include <stream.h>
#include <String.h>


void	main( void )
{
//------------------------------------------------------------------------------
//  Routine:	xx		Purpose: zzz
//  Processing NOTES:  
//  Input:  
//  Method:	
//  Last Revised:  mm/dd/1996	LS Weber
//------------------------------------------------------------------------------
//  Regex	r="\\(\\(\\([0-9]{1,3}\\(\",\"[0-9]{3}\\)*\\)+\\)\\(\".\"[0-9]{2}\\)?\\)\\|\\(\".\"[0-9]{2}\\)";

Regex	r="\\(\\([0-9]?[0-9]?[0-9]\\)\\(,[0-9][0-9][0-9]\\)*\\(\\(\\.[0-9][0-9]\\)?\\|\\.\\)\\)\\|\\(\\.[0-9][0-9]\\)";

Regex	r2="\\$?\\([0-9]?[0-9]?[0-9]\\)\\(,?[0-9][0-9][0-9]\\)*\\.[0-9][0-9]";

String	lswin;

	for( ; true ; )
	{
 		cin >> lswin;
		cout << "input: " << lswin << " Len:" << lswin.length() <<"\n";

	/*
		if(lswin.matches(r))
			cout << lswin << " is a valid number entry for Regex r\n";
		else
			cout << lswin << " is invalid\n";
	*/
		if(lswin.matches(r2))
			cout << lswin << " is a valid number entry for Regex r2\n";
		else
			cout << lswin << " is invalid for r2\n";
	}
	cout << "bye guy\n";
	
}  // End of main
	
