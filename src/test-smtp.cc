#include <String.h>

extern int SendMail(String mailserver, String to, String from, String subj,
	String data);

main()
{
	SendMail("raja", "holtrf", "<holtrf@test.com>", "crapola 5",
		"This is test numero cinco and stuff.\n-russell");
}


