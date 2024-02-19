# C++ Database Adapter implementation with PostgreSQL

This repository implements the interface for the [C++ Database Adapter](https://github.com/systelab/cpp-db-adapter) using [PostgreSQL](https://www.postgresql.org/).


## Setup

### Download using Conan

This library is designed to be installed by making use of [Conan](https://conan.io/) package manager. So, you just need to add the following requirement into your Conan recipe:

```python
def requirements(self):
   self.requires("DbPostgreSQLAdapter/1.0.0@systelab/stable")
```

> Version number of this code snipped is set just as an example. Replace it for the desired version package to retrieve.

As this package is not available on the conan-center, you will also need to configure a remote repository before installing dependencies:

```bash
conan remote add systelab-public https://artifactory.systelab.net/artifactory/api/conan/cpp-conan-production-local
```

See Conan [documentation](https://docs.conan.io/en/latest/) for further details on how to integrate this package with your build system.

### Build from sources

See [BUILD.md](BUILD.md) document for details.


## Usage

Establish connection to a SQLite database by creating a configuration object that specifies the path of .db file to open:

```cpp
#include "DbPostgreSQLAdapter/Connection.h"
#include "DbPostgreSQLAdapter/ConnectionConfiguration.h"

const std::string dbName = "dummyDB";
const std::string dbHost = "localhost";
const std::string dbUser = "User";
const std::string dbPassword = "Password";
const std::string dbPort = "5432";

static const ConnectionConfiguration defaultConfiguration(dbUser, dbPassword, dbHost, dbPort, dbName);
std::unique_ptr<systelab::db::IDatabase> database = systelab::db::postgresql::Connection().loadDatabase(configuration);
```

Use the created systelab::db::IDatabase object to access to the database as described on C++ Database Adapter documentation.
