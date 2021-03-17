#ifndef SELF_H
#define SELF_H

__BEGIN_DECLS

typedef char temp_t[6];

extern temp_t self;

int __close(int fd);

int __link_open(temp_t id);

int __reopen(const temp_t id);

int __unlink(const temp_t id);

__END_DECLS

#endif // SELF_H
