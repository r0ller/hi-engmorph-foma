clang++ -g -fstandalone-debug -std=c++11 -I/usr/pkg/include -I/home/r0ller/hi/alice -L/usr/pkg/lib -lsqlite3 extract_db_injects.cpp /home/r0ller/hi/alice/logger.cpp /home/r0ller/hi/alice/sqlite_db.cpp /home/r0ller/hi/alice/query_result.cpp -o extract_db_injects
