# Changelog
**Project mistakenly started as v0.0.1, so the second release became v1.0.0 to become SemVer compliant.**  
**Dates are expressed in dd-MM-yyyy**

### Release: v1.0.0
    Who: rafaelsilverioit (https://github.com/rafaelsilverioit)
    Date: 18-06-2021
    Description:
    * Major refactoring of files to be compatible with C++ style and to be more maintainable;
    * Implemented consistent hashing to shard requests for suitable peers (ref: https://github.com/rafaelsilverioit/sharder);
    * If there's a log_file key in proxy.conf, then we log everything in there.

### Release: v1.1.0
    Who: rafaelsilverioit (https://github.com/rafaelsilverioit)
    Date: 07-07-2021
    Description:
    * Getting rid of forking code as io_service takes care of concurrency for us.

### Release: v1.2.0
    Who: rafaelsilverioit (https://github.com/rafaelsilverioit)
    Date: 29-12-2021
    Description:
    * Avoiding process termination in case of fatal errors.

