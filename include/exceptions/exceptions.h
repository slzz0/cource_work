#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

class ScholarshipException : public std::runtime_error {
public:
    explicit ScholarshipException(const std::string& message)
        : std::runtime_error("Scholarship System Error: " + message) {}
};

class FileIOException : public ScholarshipException {
public:
    explicit FileIOException(const std::string& message)
        : ScholarshipException("File I/O Error: " + message) {}
};

class FileNotFoundException : public FileIOException {
public:
    explicit FileNotFoundException(const std::string& filename)
        : FileIOException("File not found: " + filename) {}
};

class FileWriteException : public FileIOException {
public:
    explicit FileWriteException(const std::string& filename)
        : FileIOException("Cannot write to file: " + filename) {}
};

class ParseException : public ScholarshipException {
public:
    explicit ParseException(const std::string& message)
        : ScholarshipException("Parse Error: " + message) {}
};

class InvalidDataFormatException : public ParseException {
public:
    explicit InvalidDataFormatException(const std::string& details)
        : ParseException("Invalid data format: " + details) {}
};

class ValidationException : public ScholarshipException {
public:
    explicit ValidationException(const std::string& message)
        : ScholarshipException("Validation Error: " + message) {}
};

class InvalidStudentDataException : public ValidationException {
public:
    explicit InvalidStudentDataException(const std::string& field)
        : ValidationException("Invalid student data in field: " + field) {}
};

class InvalidGradeException : public ValidationException {
public:
    explicit InvalidGradeException(double grade)
        : ValidationException("Invalid grade value: " + std::to_string(grade) + 
                            " (must be between 0.0 and 10.0)") {}
};

class InvalidCourseException : public ValidationException {
public:
    explicit InvalidCourseException(int course)
        : ValidationException("Invalid course: " + std::to_string(course) + 
                            " (must be between 1 and 4)") {}
};

class InvalidSemesterException : public ValidationException {
public:
    explicit InvalidSemesterException(int semester)
        : ValidationException("Invalid semester: " + std::to_string(semester) + 
                            " (must be between 1 and 8)") {}
};

class EmptyNameException : public ValidationException {
public:
    EmptyNameException()
        : ValidationException("Student name cannot be empty") {}
};

class DatabaseException : public ScholarshipException {
public:
    explicit DatabaseException(const std::string& message)
        : ScholarshipException("Database Error: " + message) {}
};

class StudentNotFoundException : public DatabaseException {
public:
    explicit StudentNotFoundException(const std::string& identifier)
        : DatabaseException("Student not found: " + identifier) {}
};

class DuplicateStudentException : public DatabaseException {
public:
    explicit DuplicateStudentException(const std::string& name)
        : DatabaseException("Student already exists: " + name) {}
};

#endif