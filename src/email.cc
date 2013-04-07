// email.cc
//
// PURPOSE:
//    send some email by opening a socket connection to
//    the smtp port on a mailserver. Adds a subject line.
//
// USAGE:
//
// PROGRAMMING NOTES:
//
// HISTORY:
//		1/10-11/96 Russell Holt
//
// $Id: email.cc,v 1.1 1996/03/18 13:24:05 holtrf Exp $
// Copyright 1995 by Destiny Software Corporation.

#include "Socket.h"
#include "destiny.h"

extern "C" {
	int gethostname(char *, int);
}

int SendMail(String mailserver, String to, String from, String subj,
	String data);

int read_response(Socket &s);

// SendMail
//  -- send some email
//
// mailserver: specifies the machine to connect to (port 25)
// to:         the recipient mail address
// from:       sender's mail address
// subject:    makes a subject MIME header
// data:       message body
// -RFH
int SendMail(String mailserver, String to, String from, String subj,
	String data)
{
	Socket sock(25);
	String endl = "\r\n";
	
	if (sock.Connect(mailserver.chars()) == FAIL)
		return FAIL;

char *hostnm = new char[256];

	int err = gethostname(hostnm, 256);
	if (err)
	{
		delete hostnm;
		hostnm = "nowhere";	// probably should fall out and exit.
	}

	sock << "HELO " << hostnm << endl;
		read_response(sock);
	sock << "MAIL FROM:" << from << endl;
		read_response(sock);
	sock << "RCPT TO:" << to << endl;
 		read_response(sock);
	sock << "DATA\r\n";
		read_response(sock);
	sock << "Subject: " << subj << endl;
		read_response(sock);
	sock << data << endl;
		read_response(sock);
	sock << "\r\n.\r\n";
		read_response(sock);
    sock << "QUIT\r\n";
		read_response(sock);
    sock.Close();
	    
    return SUCCEED;
}

// read_response
// just read a line from the socket.
// - It would be good to return the response code (or pass in
//   an expected response code and return a boolean-type value)
int read_response(Socket &s)
{
	char x[512];
	
//	if (
		s.readline(x, 512);
//	)
//		cout << "Got: " << x << '\n';

	return 1;
}
