# Ume Datastore #

Many large-scale simulation codes store simulation data in some form
of database.  Some reasons for doing so are:

1. This consolidates data for state save (restart) I/O into one
   structure. 
2. The in-memory storage requirements can adjust for the physics
   modules being used.
3. Allows for more nuanced access control for data.

Unfortunately, using this type of structure introduces some
inefficiencies: 

1. Compared to directly accessing arrays, there are (many) more
   instructions required to locate and access data, such as name
   lookups and data structure traversals.
2. Data references become more opaque to compile-time optimization.

We have elected to add a datastore to Ume in order to present data
motion and instruction mix models that are more representative of
actual simulation codes. The `Ume::Datastore` implements a
hierarchical key-value datastore.
