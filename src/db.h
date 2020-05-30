#ifndef _DB_H
#define _DB_H

void loaduserdb (void);
void loadchandb (void);
void loadtrustdb (void);
void loadlinkdb (void);
void loadbotservdb (void);
void saveuserdb (void);
void savechandb (void);
void savetrustdb (void);
void savelinkdb (void);
void savebotservdb (void);
void savealldb (void);
void loadalldb (void);

int connect_to_db (void);
int reconnect_to_db (void);

#endif  // _DB_H
