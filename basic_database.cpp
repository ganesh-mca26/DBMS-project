/*
    BASIC DATABASE SYSTEM IN C++ (single-file learning project)
    -----------------------------------------------------------
    Goal:
      Demonstrate table creation and record insert, view, update and delete
      using an object-oriented design and the C++ Standard Library (STL).

    Data model:
      Database owns multiple Table objects.
      Each Table stores column names and rows; each row is a vector of strings.
      This keeps the example simple and supports any number of text columns.
      Values are entered as text; data types and disk persistence are outside
      this beginner project, as described under future extensions in the brief.

    How to build:
      g++ -std=c++17 -Wall -Wextra -pedantic basic_database.cpp -o basic_database
      Windows (MinGW): g++ -std=c++17 -Wall -Wextra -pedantic basic_database.cpp -o basic_database.exe

    How to use:
      1. Create a table and enter its name and comma-separated column names.
      2. Choose Insert to add a row (enter one value for each column).
      3. Choose View to see all rows and their row numbers.
      4. Choose Update or Delete and provide the displayed row number.
      5. Choose View again to check the result; Exit closes the program.

    Design / operations:
      - createTable: validates names and adds an empty table.
      - insertRecord: gathers one value per column and appends a row.
      - viewRecords: prints column headers and all stored rows.
      - updateRecord: replaces every value in one selected row.
      - deleteRecord: removes one selected row.
      - Database::findTable: finds a table by name; returns nullptr if absent.

    Limits: Data lives only while the program runs. All values are strings.
    File storage, typed columns, SQL/query syntax and advanced constraints are
    optional future improvements, not needed for this core assignment.
*/

#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// One row contains one text value for each column in its table.
using Row = vector<string>;

class Table {
public:
    Table(string tableName, vector<string> columnNames)
        : name(move(tableName)), columns(move(columnNames)) {}

    const string& getName() const { return name; }
    const vector<string>& getColumns() const { return columns; }
    const vector<Row>& getRows() const { return rows; }

    void addRow(Row row) { rows.push_back(move(row)); }

    bool replaceRow(size_t rowIndex, Row row) {
        if (rowIndex >= rows.size()) return false;
        rows[rowIndex] = move(row);
        return true;
    }

    bool removeRow(size_t rowIndex) {
        if (rowIndex >= rows.size()) return false;
        rows.erase(rows.begin() + static_cast<ptrdiff_t>(rowIndex));
        return true;
    }

private:
    string name;
    vector<string> columns;
    vector<Row> rows;
};

class Database {
public:
    // Return an existing table by name, or nullptr when it does not exist.
    Table* findTable(const string& tableName) {
        for (Table& table : tables) {
            if (table.getName() == tableName) return &table;
        }
        return nullptr;
    }

    const vector<Table>& getTables() const { return tables; }

    bool addTable(const string& tableName, const vector<string>& columns) {
        if (findTable(tableName) != nullptr) return false;
        tables.emplace_back(tableName, columns);
        return true;
    }

private:
    vector<Table> tables;
};

// Read a full line so names and values may contain spaces.
string readLine(const string& prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

// Convert comma-separated column names into a clean list.
vector<string> parseColumns(const string& input) {
    vector<string> columns;
    string current;
    for (char character : input) {
        if (character == ',') {
            if (!current.empty()) columns.push_back(current);
            current.clear();
        } else if (!(current.empty() && character == ' ')) {
            current += character;
        }
    }
    if (!current.empty()) columns.push_back(current);
    return columns;
}

bool hasDuplicate(const vector<string>& values) {
    for (size_t i = 0; i < values.size(); ++i) {
        for (size_t j = i + 1; j < values.size(); ++j) {
            if (values[i] == values[j]) return true;
        }
    }
    return false;
}

// Ask which table to work with. The returned pointer is valid until a new
// table is added; the menu never keeps it across table creation.
Table* chooseTable(Database& database) {
    if (database.getTables().empty()) {
        cout << "No tables yet. Choose 1 to create a table first.\n";
        return nullptr;
    }

    cout << "Available tables: ";
    for (size_t i = 0; i < database.getTables().size(); ++i) {
        if (i != 0) cout << ", ";
        cout << database.getTables()[i].getName();
    }
    cout << '\n';

    const string tableName = readLine("Table name: ");
    Table* table = database.findTable(tableName);
    if (table == nullptr) cout << "Table not found. Check the name and try again.\n";
    return table;
}

void createTable(Database& database) {
    const string name = readLine("New table name: ");
    if (name.empty()) {
        cout << "Table name cannot be empty.\n";
        return;
    }
    if (database.findTable(name) != nullptr) {
        cout << "A table with that name already exists.\n";
        return;
    }

    cout << "Enter column names separated by commas (example: id, name, age).\n";
    vector<string> columns = parseColumns(readLine("Columns: "));
    if (columns.empty()) {
        cout << "A table must have at least one column.\n";
        return;
    }
    if (hasDuplicate(columns)) {
        cout << "Column names must be unique.\n";
        return;
    }

    database.addTable(name, columns);
    cout << "Table '" << name << "' created with " << columns.size() << " column(s).\n";
}

void insertRecord(Database& database) {
    Table* table = chooseTable(database);
    if (table == nullptr) return;

    Row row;
    row.reserve(table->getColumns().size());
    for (const string& column : table->getColumns()) {
        row.push_back(readLine("Value for " + column + ": "));
    }
    table->addRow(move(row));
    cout << "Record inserted.\n";
}

void viewRecords(Database& database) {
    Table* table = chooseTable(database);
    if (table == nullptr) return;

    cout << "\nTable: " << table->getName() << '\n';
    for (const string& column : table->getColumns()) cout << "| " << column << ' ';
    cout << "|\n";
    for (size_t i = 0; i < table->getColumns().size(); ++i) cout << "------------";
    cout << "\n";

    if (table->getRows().empty()) {
        cout << "(No records yet)\n";
        return;
    }

    for (size_t rowIndex = 0; rowIndex < table->getRows().size(); ++rowIndex) {
        cout << rowIndex + 1 << ". ";
        for (const string& value : table->getRows()[rowIndex]) cout << "| " << value << ' ';
        cout << "|\n";
    }
}

bool readRowNumber(size_t& rowIndex) {
    cout << "Row number: ";
    long long entered = 0;
    if (!(cin >> entered)) {
        cout << "Please enter a valid whole number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (entered < 1) {
        cout << "Row numbers start at 1.\n";
        return false;
    }
    rowIndex = static_cast<size_t>(entered - 1);
    return true;
}

void updateRecord(Database& database) {
    Table* table = chooseTable(database);
    if (table == nullptr) return;
    if (table->getRows().empty()) {
        cout << "There are no records to update.\n";
        return;
    }
    cout << "Use the row number shown by View records.\n";
    size_t rowIndex = 0;
    if (!readRowNumber(rowIndex)) return;
    if (rowIndex >= table->getRows().size()) {
        cout << "That row number does not exist.\n";
        return;
    }

    Row updatedRow;
    updatedRow.reserve(table->getColumns().size());
    for (const string& column : table->getColumns()) {
        updatedRow.push_back(readLine("New value for " + column + ": "));
    }
    table->replaceRow(rowIndex, move(updatedRow));
    cout << "Record updated.\n";
}

void deleteRecord(Database& database) {
    Table* table = chooseTable(database);
    if (table == nullptr) return;
    if (table->getRows().empty()) {
        cout << "There are no records to delete.\n";
        return;
    }
    cout << "Use the row number shown by View records.\n";
    size_t rowIndex = 0;
    if (!readRowNumber(rowIndex)) return;
    if (!table->removeRow(rowIndex)) {
        cout << "That row number does not exist.\n";
        return;
    }
    cout << "Record deleted.\n";
}

int main() {
    Database database;

    cout << "=== Basic Database System (C++) ===\n";
    cout << "Data is kept in memory and will be cleared when you exit.\n";

    while (true) {
        cout << "\nMenu\n"
             << "1. Create table\n"
             << "2. Insert record\n"
             << "3. View records\n"
             << "4. Update record\n"
             << "5. Delete record\n"
             << "0. Exit\n";
        cout << "Choose an operation: ";

        int choice = -1;
        if (!(cin >> choice)) {
            if (cin.eof()) break;
            cout << "Please enter a menu number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1: createTable(database); break;
            case 2: insertRecord(database); break;
            case 3: viewRecords(database); break;
            case 4: updateRecord(database); break;
            case 5: deleteRecord(database); break;
            case 0:
                cout << "Goodbye.\n";
                return 0;
            default:
                cout << "Choose one of the listed menu options.\n";
        }
    }
    return 0;
}
