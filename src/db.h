#ifndef _DB_H
#define _DB_H

void saveuserdb (void);
void savechandb (void);
void savetrustdb (void);
void savelinkdb (void);
void savebotservdb (void);
void savealldb (void);

int connect_to_db (void);

#endif  // _DB_H
