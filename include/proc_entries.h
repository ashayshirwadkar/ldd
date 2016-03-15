#ifndef _PROC_ENTRIES_H_
#define _PROC_ENTRIES_H_

#include <linux/proc_fs.h>

#define MAXLEN 256

void create_proc_entries(void);
void remove_proc_entries(void);

#endif /* _PROC_ENTRIES_H_ */
