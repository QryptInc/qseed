#ifndef COMMON_H
#define COMMON_H

#include <string>

/// <summary>
/// Removes trailing whitespace in a string
/// </summary>
///
/// <param name="str">The input string to modify</param>
/// <returns>String with trailing whitespace removed</returns>
std::string trimWhitespace(std::string str);

/// <summary>
/// Timestamp the current date and time into a string 
/// </summary>
///
/// <returns>Timestamp string</returns>
std::string getTimestamp();

/// <summary>
/// Log a message to stdout
///
/// This function will prepend a timestamp
/// </summary>
///
/// <param name="message">The string message</param>
void infoLog(std::string message);

#endif