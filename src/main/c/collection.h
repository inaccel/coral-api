#ifndef COLLECTION_H
#define COLLECTION_H

__BEGIN_DECLS

void __clear(void ***collection);

int __set(void ***collection, void *item);

void __unset(void ***collection, void *item);

__END_DECLS

#endif // COLLECTION_H
