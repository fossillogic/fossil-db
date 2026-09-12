# CrabDB

CrabDB is a lightweight, embedded C database library with file-backed and in-memory database lifecycle management, table and record operations, typed values, transactions, result containers, and status reporting. Its API uses explicit ownership, dynamically sized collections, and pointer-free file persistence.

## Key Features

- Version and human-readable status-string APIs
- File-backed database creation, opening, closing, and destruction
- In-memory database creation for temporary, non-file-backed use
- Database last-error access with explicit status reporting
- Dynamic table management, including create, drop, rename, and existence checks
- Record insertion, update, deletion, and table selection
- Result containers with record counts and cleanup
- Typed value creation, type inspection, and destruction
- Transaction begin, commit, and rollback support for table changes
- Status codes for invalid arguments, allocation failures, missing or existing objects, I/O errors, corruption, read-only state, and transaction or query failures

## ***Prerequisites***

To get started, ensure you have the following installed:

- **Meson Build System**: If you don’t have Meson `1.10.0` or newer installed, follow the installation instructions on the official [Meson website](https://mesonbuild.com/Getting-meson.html).

### Adding Dependency

#### Adding via Meson Git Wrap

To add a git-wrap, place a `.wrap` file in `subprojects` with the Git repo URL and revision, then use `dependency('fossil-db')` in `meson.build` so Meson can fetch and build it automatically.

#### Integrate the Dependency:

Add the `fossil-db.wrap` file in your `subprojects` directory and include the following content:

```ini
[wrap-git]
url = https://github.com/fossillogic/fossil-db.git
revision = v1.0.2

[provide]
dependency_names = fossil-db
```

**Note**: For the best experience, always use the latest releases. Visit the [releases](https://github.com/fossillogic/fossil-crabdb/releases) page for the latest versions.

## Configure Options

You have options when configuring the build, each serving a different purpose:

- **Running Tests**: To enable running tests, use `-Dwith_test=enabled` when configuring the build.

Example:

```sh
meson setup builddir -Dwith_test=enabled
```

### Tests Double as Samples

The project is designed so that **test cases serve two purposes**:

- ✅ **Unit Tests** – validate the framework’s correctness.  
- 📖 **Usage Samples** – demonstrate how to use these libraries through test cases.  

This approach keeps the codebase compact and avoids redundant “hello world” style examples.  
Instead, the same code that proves correctness also teaches usage.  

This mirrors the **Meson build system** itself, which tests its own functionality by using Meson to test Meson.  
In the same way, Fossil Logic validates itself by demonstrating real-world usage in its own tests via Fossil Test.  

```bash
meson test -C builddir -v
```

Running the test suite gives you both verification and practical examples you can learn from.

## Contributing and Support

If you're interested in contributing to this project, encounter any issues, have questions, or would like to provide feedback, don't hesitate to open an issue or visit the [Fossil Logic Docs](https://fossillogic.com/docs) for more information.
