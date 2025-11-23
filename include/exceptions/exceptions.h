#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <format>
#include <stdexcept>
#include <string>

class ScholarshipException : public std::runtime_error {
public:
    explicit ScholarshipException(const std::string& message)
        : std::runtime_error(std::format("Scholarship System Error: {}", message)) {}
};

class FileIOException : public ScholarshipException {
public:
    explicit FileIOException(const std::string& message)
        : ScholarshipException(std::format("File I/O Error: {}", message)) {}
};

class FileNotFoundException : public FileIOException {
public:
    explicit FileNotFoundException(const std::string& filename)
        : FileIOException(std::format("File not found: {}", filename)) {}
};

class FileWriteException : public FileIOException {
public:
    explicit FileWriteException(const std::string& filename)
        : FileIOException(std::format("Cannot write to file: {}", filename)) {}
};

class ParseException : public ScholarshipException {
public:
    explicit ParseException(const std::string& message)
        : ScholarshipException(std::format("Parse Error: {}", message)) {}
};

class InvalidDataFormatException : public ParseException {
public:
    explicit InvalidDataFormatException(const std::string& details)
        : ParseException(std::format("Invalid data format: {}", details)) {}
};

class ValidationException : public ScholarshipException {
public:
    explicit ValidationException(const std::string& message)
        : ScholarshipException(std::format("Validation Error: {}", message)) {}
};

class InvalidStudentDataException : public ValidationException {
public:
    explicit InvalidStudentDataException(const std::string& field)
        : ValidationException(std::format("Invalid student data in field: {}", field)) {}
};

class InvalidGradeException : public ValidationException {
public:
    explicit InvalidGradeException(double grade)
        : ValidationException(std::format("Invalid grade value: {} (must be between 0.0 and 10.0)", grade)) {}
};

class InvalidCourseException : public ValidationException {
public:
    explicit InvalidCourseException(int course)
        : ValidationException(std::format("Invalid course: {} (must be between 1 and 4)", course)) {}
};

class InvalidSemesterException : public ValidationException {
public:
    explicit InvalidSemesterException(int semester)
        : ValidationException(std::format("Invalid semester: {} (must be between 1 and 8)", semester)) {}
};

class EmptyNameException : public ValidationException {
public:
    EmptyNameException()
        : ValidationException("Student name cannot be empty") {}
};

class DatabaseException : public ScholarshipException {
public:
    explicit DatabaseException(const std::string& message)
        : ScholarshipException(std::format("Database Error: {}", message)) {}
};

class StudentNotFoundException : public DatabaseException {
public:
    explicit StudentNotFoundException(const std::string& identifier)
        : DatabaseException(std::format("Student not found: {}", identifier)) {}
};

class DuplicateStudentException : public DatabaseException {
public:
    explicit DuplicateStudentException(const std::string& name)
        : DatabaseException(std::format("Student already exists: {}", name)) {}
};

#endif