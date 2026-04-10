# TexLoom — Coding Conventions

This document defines the coding style and naming conventions for the TexLoom project.

## File Naming

### Source Files

- **C++ Classes**: `PascalCase.h` / `PascalCase.cpp`
  - Example: `ProjectModel.h`, `ConversionEngine.cpp`
- **Headers and implementation in the same directory**
  - `src/core/ProjectModel.h`
  - `src/core/ProjectModel.cpp`

### Special Files

- `main.cpp` — Application entry point (lowercase)
- `CMakeLists.txt` — Build configuration (standard CMake naming)

## Code Naming

### Namespaces

All code is wrapped in the `texloom` namespace:

```cpp
namespace texloom {

class ProjectModel {
    // ...
};

} // namespace texloom
```

**Rationale**: Avoids global namespace pollution while keeping code clean.

### Classes and Structs

- **PascalCase** (capitalize first letter of each word)
- Descriptive names that indicate purpose

```cpp
class ProjectModel { };
class ConversionEngine { };
class EditorComponent { };
class LatexCompiler { };
```

### Methods and Functions

- **camelCase** (lowercase first letter, capitalize subsequent words)
- Verb-based names for actions

```cpp
void loadProject(const QString& path);
bool convertToLatex();
QString getCurrentDocument() const;
void setActiveFile(const QString& path);
```

### Variables

**Local variables**: `camelCase`

```cpp
QString filePath;
int documentCount;
QList<Document*> activeDocuments;
```

**Member variables**: `m_` prefix + `camelCase`

```cpp
class ProjectModel {
private:
    QString m_projectPath;
    QList<Document*> m_documents;
    bool m_isModified;
};
```

**Rationale**: The `m_` prefix clearly distinguishes member variables from local variables and parameters.

**Constants**: `kPascalCase` or `UPPER_SNAKE_CASE` for macros

```cpp
const int kDefaultFontSize = 11;
const QString kDefaultTemplate = "article";

#define TEXLOOM_VERSION "1.0.0"
```

### Enums

- **Enum name**: `PascalCase`
- **Enum values**: `PascalCase` (not UPPER_CASE)

```cpp
enum class DocumentType {
    Markdown,
    LaTeX,
    BibTeX
};

enum class ConversionStatus {
    Idle,
    Converting,
    Success,
    Failed
};
```

**Use `enum class`** (C++11 scoped enums) for type safety.

## Qt-specific Conventions

### Signals and Slots

- **Signals**: Past tense or noun form

```cpp
signals:
    void documentChanged(Document* doc);
    void conversionFinished(const QString& pdfPath);
    void errorOccurred(const QString& message);
```

- **Slots**: Action verbs (similar to methods)

```cpp
private slots:
    void onDocumentChanged();
    void handleConversionFinished(const QString& pdfPath);
    void showError(const QString& message);
```

**Prefix slots with `on` or `handle`** to distinguish them from regular methods (optional but recommended).

### Q_OBJECT Macro

Always include at the top of QObject-derived classes:

```cpp
class ProjectModel : public QObject {
    Q_OBJECT
    
public:
    // ...
};
```

## Code Formatting

### Indentation

- **4 spaces** (no tabs)
- Qt code style standard

### Braces

- **Opening brace on same line** (K&R style)

```cpp
void myFunction() {
    if (condition) {
        doSomething();
    } else {
        doSomethingElse();
    }
}
```

### Line Length

- **Maximum 100-120 characters per line**
- Break long lines at logical points

### Pointer and Reference Alignment

- **Align with type**, not variable:

```cpp
QString* pointer;      // Recommended
QString& reference;

// Not:
QString *pointer;
QString &reference;
```

### Header Guards vs #pragma once

Use **`#pragma once`** for header files (modern, simpler):

```cpp
#pragma once

namespace texloom {

class MyClass {
    // ...
};

} // namespace texloom
```

## Comments

### Documentation Comments

Use **Doxygen-style** comments for public APIs:

```cpp
/**
 * @brief Loads a project from the specified path
 * @param path Absolute path to the .texloom project file
 * @return true if the project was loaded successfully, false otherwise
 */
bool loadProject(const QString& path);
```

### Inline Comments

- Use `//` for single-line comments
- Explain **why**, not **what**

```cpp
// Cache the result to avoid repeated conversions
if (!m_cachedPdf.isEmpty()) {
    return m_cachedPdf;
}
```

## File Organization

### Header File Structure

```cpp
#pragma once

#include <QObject>
#include <QString>

namespace texloom {

/**
 * @brief Brief description
 * 
 * Detailed description (optional)
 */
class MyClass : public QObject {
    Q_OBJECT
    
public:
    // Constructors
    MyClass(QObject* parent = nullptr);
    ~MyClass() override;
    
    // Public methods
    void publicMethod();
    
signals:
    void mySignal();
    
public slots:
    void mySlot();
    
private slots:
    void onInternalEvent();
    
private:
    // Private methods
    void privateMethod();
    
    // Member variables
    QString m_memberVariable;
};

} // namespace texloom
```

### Source File Structure

```cpp
#include "MyClass.h"

#include <QDebug>

namespace texloom {

MyClass::MyClass(QObject* parent)
    : QObject(parent)
    , m_memberVariable("default")
{
    // Constructor implementation
}

MyClass::~MyClass() {
    // Destructor implementation
}

void MyClass::publicMethod() {
    // Implementation
}

} // namespace texloom
```

## Include Order

1. **Corresponding header** (for .cpp files)
2. **C++ standard library** (`<vector>`, `<string>`, etc.)
3. **Qt headers** (`<QObject>`, `<QString>`, etc.)
4. **Project headers** (`"ProjectModel.h"`, etc.)

```cpp
#include "MyClass.h"

#include <string>
#include <vector>

#include <QObject>
#include <QString>

#include "ProjectModel.h"
#include "Document.h"
```

**Separate groups with blank lines.**

## Memory Management

### Qt Object Ownership

- Use **Qt parent-child ownership** for QObject-derived classes
- Parent objects automatically delete their children

```cpp
auto* widget = new MyWidget(parentWidget);
// No manual delete needed, parent will handle it
```

### Smart Pointers

For non-QObject classes, prefer **smart pointers**:

```cpp
std::unique_ptr<MyClass> obj = std::make_unique<MyClass>();
std::shared_ptr<MyClass> shared = std::make_shared<MyClass>();
```

### Raw Pointers

Avoid `new`/`delete` for manual memory management unless necessary.

## Error Handling

### Return Values

- Use **bool** for success/failure
- Use **std::optional** or nullable types for optional values

```cpp
bool loadFile(const QString& path);
std::optional<QString> findTemplate(const QString& name);
```

### Exceptions

- **Avoid exceptions in Qt code** (Qt uses return values and signals)
- Handle errors via return codes or error signals

```cpp
signals:
    void errorOccurred(const QString& message);
```

## Naming Examples Summary

| Element | Convention | Example |
|---------|------------|---------|
| Files | `PascalCase.h/.cpp` | `ProjectModel.cpp` |
| Classes | `PascalCase` | `ConversionEngine` |
| Methods | `camelCase` | `loadProject()` |
| Variables | `camelCase` | `fileName` |
| Members | `m_camelCase` | `m_projectPath` |
| Constants | `kPascalCase` | `kDefaultMargin` |
| Enums | `PascalCase` | `DocumentType` |
| Enum values | `PascalCase` | `DocumentType::Markdown` |
| Namespace | `lowercase` | `texloom` |
| Signals | past tense | `documentChanged()` |
| Slots | action verb | `onDocumentChanged()` |

## Tools

### Code Formatting

**ClangFormat** configuration (`.clang-format`):

```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
PointerAlignment: Left
```

Run: `clang-format -i src/**/*.cpp src/**/*.h`

### Static Analysis

- **clang-tidy** for C++ best practices
- **Cppcheck** for additional checks

## C++ Version

- **C++20** minimum
- Use modern C++ features:
  - Range-based for loops
  - Auto type deduction (where clear)
  - Lambda expressions
  - Smart pointers
  - Structured bindings

```cpp
// Modern C++20 style
for (const auto& doc : documents) {
    if (auto path = doc->filePath(); !path.isEmpty()) {
        processFile(path);
    }
}
```

## Additional Resources

- [Qt Coding Conventions](https://wiki.qt.io/Qt_Coding_Style)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
