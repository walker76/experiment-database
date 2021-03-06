#include "stdafx.h"
#include <iostream>
#include <mysqlx/xdevapi.h>
#include <string>
#include <ctime>
#include <iomanip>
#include <regex>
#include <fstream>

using namespace std;
using namespace mysqlx;

int initializeSession(Session& conn);
void insertExperiment(Session& conn);
void insertRun(Session& conn);
void displayExperimentMetaData(Session& conn);
void displayRunMetaData(Session& conn);
void generateExperimentReport(Session& conn);
void generateAggregateReport(Session& conn);
void parameterSearch(Session& conn);

int main(int argc, char* argv[]) {
    try {
        SessionSettings initialOptions(
            SessionOption::USER, "HW3335",
            SessionOption::PWD, "PW3335",
            SessionOption::HOST, "localhost",
            SessionOption::SSL_MODE, SSLMode::REQUIRED
        );

        cout << "Creating session from options...";
        Session conn(initialOptions);
        cout << "success." << endl;

        int errorCode = initializeSession(conn);
        if (errorCode == 1) {
            cout << "Error intializing database schema" << endl;
            return 1;
        }

        char selectedChoice;
        std::string abandon;

        do {
            cout << "\nPlease choose an option to interact with the database:" << endl;
            cout << "1: Add an experiment" << endl;
            cout << "2: Add a run" << endl;
            cout << "3: Retrieve information about an experiment" << endl;
            cout << "4: Retrieve information about a run" << endl;
            cout << "5: Generate experiment report" << endl;
            cout << "6: Generate aggragate report" << endl;
            cout << "7: Parameter search" << endl;
            cout << "q: quit" << endl;
            cout << "> ";

            cin >> selectedChoice;
            getline(cin, abandon);

            switch (selectedChoice) {
                case '1':
                    insertExperiment(conn);
                    break;
                case '2': 
                    insertRun(conn);
                    break;
                case '3':
                    displayExperimentMetaData(conn);
                    break;
                case '4':
                    displayRunMetaData(conn);
                    break;
                case '5':
                    generateExperimentReport(conn);
                    break;
                case '6':
                    generateAggregateReport(conn);
                    break;
                case '7':
                    parameterSearch(conn);
                    break;
                case 'q':
                    cout << "q" << endl;
                    return 0;
                default:
                    cout << "Unknown Input" << endl;
            }
        } while (selectedChoice != 'q');
        
    } catch (const Error &err) {
        cout << "ERROR: " << err << endl;
        return 1;
    } catch (std::exception &ex) {
        cout << "STD EXCEPTION: " << ex.what() << endl;
        return 1;
    } catch (const char *ex) {
        cout << "EXCEPTION: " << ex << endl;
        return 1;
    }
    return 0;
}

int initializeSession(Session& conn) {
    cout << "Creating schema Experiment......";
    Schema db = conn.createSchema("Experiment", true);
    cout << "success." << endl;

    cout << "Connecting to schema............";
    conn.sql("USE Experiment").execute();
    cout << "success." << endl;

    cout << "Creating table Experiment.......";
    SqlStatement createTable = conn.sql(
        "CREATE TABLE IF NOT EXISTS Experiment (" \
        "ExperimentId VARCHAR(255) NOT NULL," \
        "ManagerID CHAR(6) NOT NULL," \
        "StartDate DATE NOT NULL," \
        "DataEntryDate DATE NOT NULL," \
        "PRIMARY KEY(ExperimentId)" \
        ") "
    );
    SqlResult resultCode = createTable.execute();
    if (resultCode.getWarningsCount() <= 0) {
        cout << "success." << endl;
    }
    else {
        cout << "failed." << endl;
        return 1;
    }

    cout << "Creating table ParamterTypes....";
    createTable = conn.sql(
        "CREATE TABLE IF NOT EXISTS ParameterTypes (" \
        "ExperimentId VARCHAR(255) NOT NULL," \
        "ParameterName VARCHAR(255) NOT NULL," \
        "Type VARCHAR(255) NOT NULL," \
        "Required BOOLEAN NOT NULL," \
        "CONSTRAINT PK_ParameterTypes PRIMARY KEY (ExperimentId,ParameterName)," \
        "FOREIGN KEY(ExperimentId) REFERENCES Experiment(ExperimentId)" \
        ") "
    );
    resultCode = createTable.execute();
    if (resultCode.getWarningsCount() <= 0) {
        cout << "success." << endl;
    }
    else {
        cout << "failed." << endl;
        return 1;
    }

    cout << "Creating table ResultTypes......";
    createTable = conn.sql(
        "CREATE TABLE IF NOT EXISTS ResultTypes (" \
        "ExperimentId VARCHAR(255) NOT NULL," \
        "ResultName VARCHAR(255) NOT NULL," \
        "Type VARCHAR(255) NOT NULL," \
        "Required BOOLEAN NOT NULL," \
        "CONSTRAINT PK_ResultTypes PRIMARY KEY (ExperimentId,ResultName)," \
        "FOREIGN KEY(ExperimentId) REFERENCES Experiment(ExperimentId)" \
        ") "
    );
    resultCode = createTable.execute();
    if (resultCode.getWarningsCount() <= 0) {
        cout << "success." << endl;
    }
    else {
        cout << "failed." << endl;
        return 1;
    }

    cout << "Creating table Runs.............";
    createTable = conn.sql(
        "CREATE TABLE IF NOT EXISTS Runs (" \
        "ExperimentId VARCHAR(255) NOT NULL," \
        "TimeOfRun DATETIME NOT NULL," \
        "ExperimenterSSN CHAR(6) NOT NULL," \
        "Success BOOLEAN NOT NULL," \
        "CONSTRAINT PK_Runs PRIMARY KEY (ExperimentId,TimeOfRun)," \
        "FOREIGN KEY(ExperimentId) REFERENCES Experiment(ExperimentId)" \
        ") "
    );
    resultCode = createTable.execute();
    if (resultCode.getWarningsCount() <= 0) {
        cout << "success." << endl;
    }
    else {
        cout << "failed." << endl;
        return 1;
    }

    cout << "Creating table RunsParameter....";
    createTable = conn.sql(
        "CREATE TABLE IF NOT EXISTS RunsParameter (" \
        "ExperimentId VARCHAR(255) NOT NULL," \
        "TimeOfRun DATETIME NOT NULL," \
        "ParameterName VARCHAR(255) NOT NULL," \
        "Value VARCHAR(255) NOT NULL," \
        "CONSTRAINT PK_RunsParameter PRIMARY KEY (ExperimentId,TimeOfRun,ParameterName)," \
        "FOREIGN KEY(ExperimentId, TimeOfRun) REFERENCES Runs(ExperimentId, TimeOfRun)," \
        "FOREIGN KEY(ExperimentId, ParameterName) REFERENCES ParameterTypes(ExperimentId, ParameterName)" \
        ") "
    );
    resultCode = createTable.execute();
    if (resultCode.getWarningsCount() <= 0) {
        cout << "success." << endl;
    }
    else {
        cout << "failed." << endl;
        return 1;
    }

    cout << "Creating table RunsResult.......";
    createTable = conn.sql(
        "CREATE TABLE IF NOT EXISTS RunsResult (" \
        "ExperimentId VARCHAR(255) NOT NULL," \
        "TimeOfRun DATETIME NOT NULL," \
        "ResultName VARCHAR(255) NOT NULL," \
        "Value VARCHAR(255) NOT NULL," \
        "CONSTRAINT PK_RunsResult PRIMARY KEY (ExperimentId,TimeOfRun,ResultName)," \
        "FOREIGN KEY(ExperimentId) REFERENCES Experiment(ExperimentId)," \
        "FOREIGN KEY(ExperimentId, TimeOfRun) REFERENCES Runs(ExperimentId, TimeOfRun)," \
        "FOREIGN KEY(ExperimentId, ResultName) REFERENCES ResultTypes(ExperimentId, ResultName)" \
        ") "
    );
    resultCode = createTable.execute();
    if (resultCode.getWarningsCount() <= 0) {
        cout << "success." << endl;
    }
    else {
        cout << "failed." << endl;
        return 1;
    }

    return 0;
}

void insertExperiment(Session& conn) {
    std::string abandon;
    std::string experimentId;
    std::string managerId;
    std::string startDate;

    // Loop until a valid experiment is chosen
    int errorCode;
    do {
        // Read in the experiment ID
        cout << "Please enter an experiment ID " << endl;
        cout << ">> ";
        getline(cin, experimentId);

        // Test if the experimentId exists in the Experiment table
        SqlResult result = conn.sql("SELECT * FROM Experiment WHERE experimentId=?").bind(experimentId).execute();

        // Use 'errorCode' as the number of tuples returned
        errorCode = result.count();

        // Print an error message if it's not a valid experiment
        if (errorCode > 0) {
            cout << "Please enter a new experimentId" << endl;
        }

    } while (errorCode > 0);
    
    do {
        cout << "Please enter a manager ID " << endl;
        cout << ">> ";
        getline(cin, managerId);

        if (managerId.size() > 6) {
            cout << "The manager ID must be no greater than 6 characters" << endl;
        }

    } while (managerId.size() > 6);

    regex dateR("^[0-9]{4}-[0-9]{1,2}-[0-9]{1,2}$");

    // Loop until the input matches the date regex
    do {
        cout << "Please enter a start date for the experiment (YYYY-MM-DD)" << endl;
        cout << ">> ";
        getline(cin, startDate);
    } while (!regex_match(startDate, dateR));

    try {
        cout << "Inserting experiment " << experimentId << " - ";
        SqlStatement insertStatement = conn.sql(
            "INSERT INTO Experiment " \
            "VALUES(?,?,?,CURDATE())"
        );
        insertStatement.bind(experimentId);
        insertStatement.bind(managerId);
        insertStatement.bind(startDate);
        SqlResult resultCode = insertStatement.execute();
        if (resultCode.getWarningsCount() <= 0) {
            cout << "success." << endl;
        }
        else {
            cout << "fail." << endl;
            return;
        }
    }
    catch (const Error &err) {
        cout << err.what() << endl;
    }

    // *************PARAMETER***************
    std::string parameterName;
    std::string type;
    char ch;
    char chR;
    bool required;

    do {
        cout << "Do you have any parameter types? (y/n) " << endl;
        cout << ">> ";
        cin >> ch;
        getline(cin, abandon);

        if (ch == 'y' || ch == 'Y') {

            cout << "Are you sure?" << endl;
            cout << ">> ";
            cin >> ch;
            getline(cin, abandon);

            if (ch == 'y' || ch == 'Y') {
                cout << "Please enter a parameter name " << endl;
                cout << ">> ";
                getline(cin, parameterName);

                do {
                    // TODO - Regex for date
                    cout << "Please enter a parameter type" << endl;
                    cout << ">> ";
                    getline(cin, type);
                } while (type.compare("INT") != 0 &&
                    type.compare("FLOAT") != 0 &&
                    type.compare("STRING") != 0 &&
                    type.compare("URL") != 0 &&
                    type.compare("DATE") != 0 &&
                    type.compare("DATETIME") != 0);

                do {
                    cout << "Is this parameter required? (y/n)" << endl;
                    cin >> chR;
                    getline(cin, abandon);

                    if (chR == 'y' || chR == 'Y') {
                        required = true;
                    }
                    else if (chR == 'n' || chR == 'N') {
                        required = false;
                    }
                    else {
                        cout << "Unknown input" << endl;
                    }

                } while (chR != 'y' && chR != 'Y' && chR != 'n' && chR != 'N');

                try {
                    cout << "Inserting parameter " << parameterName << " - ";
                    SqlStatement insertStatement = conn.sql(
                        "INSERT INTO ParameterTypes " \
                        "VALUES(?,?,?,?)"
                    );
                    insertStatement.bind(experimentId);
                    insertStatement.bind(parameterName);
                    insertStatement.bind(type);
                    insertStatement.bind(required);
                    SqlResult resultCode = insertStatement.execute();
                    if (resultCode.getWarningsCount() <= 0) {
                        cout << "success." << endl;
                    }
                    else {
                        cout << "fail." << endl;
                        return;
                    }
                }
                catch (const Error &err) {
                    cout << err.what() << endl;
                }
            }
            else {
                ch = ' ';
            }
        }
        else if (ch != 'n' && ch != 'N') {
            cout << "Unknown input" << endl;
        }

    } while (ch != 'n' && ch != 'N');

    // *************RESULT***************
    std::string resultName;

    do {
        cout << "Do you have any result types? (y/n) " << endl;
        cout << ">> ";
        cin >> ch;
        getline(cin, abandon);

        if (ch == 'y' || ch == 'Y') {

            cout << "Are you sure?" << endl;
            cout << ">> ";
            cin >> ch;
            getline(cin, abandon);

            if (ch == 'y' || ch == 'Y') {

                cout << "Please enter a result name " << endl;
                cout << ">> ";
                getline(cin, resultName);

                do {
                    // TODO - Regex for date
                    cout << "Please enter a result type" << endl;
                    cout << ">> ";
                    getline(cin, type);
                } while (type.compare("INT") != 0 &&
                    type.compare("FLOAT") != 0 &&
                    type.compare("STRING") != 0 &&
                    type.compare("URL") != 0 &&
                    type.compare("DATE") != 0 &&
                    type.compare("DATETIME") != 0);

                do {
                    cout << "Is this result required? (y/n)" << endl;
                    cin >> chR;
                    getline(cin, abandon);

                    if (chR == 'y' || chR == 'Y') {
                        required = true;
                    }
                    else if (chR == 'n' || chR == 'N') {
                        required = false;
                    }
                    else {
                        cout << "Unknown input" << endl;
                    }

                } while (chR != 'y' && chR != 'Y' && chR != 'n' && chR != 'N');

                try {
                    cout << "Inserting result " << resultName << " - ";
                    SqlStatement insertStatement = conn.sql(
                        "INSERT INTO ResultTypes " \
                        "VALUES(?,?,?,?)"
                    );
                    insertStatement.bind(experimentId);
                    insertStatement.bind(resultName);
                    insertStatement.bind(type);
                    insertStatement.bind(required);
                    SqlResult resultCode = insertStatement.execute();
                    if (resultCode.getWarningsCount() <= 0) {
                        cout << "success." << endl;
                    }
                    else {
                        cout << "fail." << endl;
                        return;
                    }
                }
                catch (const Error &err) {
                    cout << err.what() << endl;
                }
            }
            else {
                ch = ' ';
            }
        }
        else if (ch != 'n' && ch != 'N') {
            cout << "Unknown input" << endl;
        }

    } while (ch != 'n' && ch != 'N');

}

void insertRun(Session& conn) {

    // Setup value regex
    regex intValueR("^[0-9]+$");
    regex floatValueR("^[0-9]+(\.[0-9]+)?$");
    regex urlValueR("^[a-zA-z]+\.[a-zA-z]+\.[a-zA-Z]{3}$");
    regex timeValueR("^[0-9]{2}:[0-9]{2}:[0-9]{2}$");

    // This string is used to flush the buffer of stdin
    std::string abandon;

    // String to hold the experimentId, will be used in all insert statements so it's declared
    // outside the main body of the insertion program
    std::string experimentId;

    // Loop until a valid experiment is chosen
    int errorCode;
    do {
        // Read in the experiment ID
        cout << "Please enter an experiment ID " << endl;
        cout << ">> ";
        getline(cin, experimentId);

        // Test if the experimentId exists in the Experiment table
        SqlResult result = conn.sql("SELECT * FROM Experiment WHERE experimentId=?").bind(experimentId).execute();

        // Use 'errorCode' as the number of tuples returned
        errorCode = result.count();

        // Print an error message if it's not a valid experiment
        if (errorCode <= 0) {
            cout << "Please enter a valid experimentId" << endl;
        }

    } while (errorCode <= 0);

    // Setup string to hold the date and the regex to test against
    std::string date;
    regex dateR("^[0-9]{4}-[0-9]{1,2}-[0-9]{1,2}$");

    // Loop until the input matches the date regex
    do {
        cout << "Please enter the date of the run (YYYY-MM-DD)" << endl;
        cout << ">> ";
        getline(cin, date);
    } while (!regex_match(date, dateR));

    // Setup string to hold the time and the regex to test against
    std::string time;
    regex timeR("^[0-9]{2}:[0-9]{2}:[0-9]{2}$");

    // Loop until the input matches the time regex
    do {
        cout << "Please enter the time of the run (HH:MM:SS)" << endl;
        cout << ">> ";
        getline(cin, time);
    } while (!regex_match(time, timeR));

    SqlResult result = conn.sql("SELECT * FROM Runs WHERE experimentId=? AND TimeOfRun=?").bind(experimentId).bind(date + " " + time).execute();
    if (result.count() >= 1) {
        cout << "You have already inserted a run for experiment " << experimentId << " at date and time " << date << " " << time << endl;
        return;
    }

    // Loop until a valid experimenter ID is given (< 6 characters)
    std::string experimenterId;
    do {
        cout << "Please enter the experimenter's ID" << endl;
        cout << ">> ";
        getline(cin, experimenterId);

        // If the experiment Id is greater than 6 characters then print an error message
        if (experimenterId.size() > 6) {
            cout << "The manager ID must be no greater than 6 characters" << endl;
        }

    } while (experimenterId.size() > 6);

    // Setup a character to hold the input and a boolean to hold the resulting success value
    char chS;
    bool success;

    // Loop until a valid answer is given
    do {
        cout << "Was this run a success? (y/n)" << endl;
        cin >> chS;
        getline(cin, abandon);

        // If it's a yes then success is true
        if (chS == 'y' || chS == 'Y') {
            success = true;
        } // If it's a no then success is false
        else if (chS == 'n' || chS == 'N') {
            success = false;
        } // If it's anything else then print an error message
        else {
            cout << "Unknown input" << endl;
        }

    } while (chS != 'y' && chS != 'Y' && chS != 'n' && chS != 'N');

    // Insert the run
    try {
        // Build the SQL query
        cout << "Inserting run...";
        SqlStatement insertStatement = conn.sql(
            "INSERT INTO Runs " \
            "VALUES(?,?,?,?)"
        );

        // Bind the values to the query
        insertStatement.bind(experimentId);
        insertStatement.bind(date + " " + time);
        insertStatement.bind(experimenterId);
        insertStatement.bind(success);

        // Execute the query and store the result
        SqlResult resultCode = insertStatement.execute();

        // If there's no warnings then it was a success
        if (resultCode.getWarningsCount() <= 0) {
            cout << "success." << endl;
        } // If there are warnings then print error message and quit
        else {
            cout << "fail." << endl;
            return;
        }
    }
    catch (const Error &err) {
        cout << err.what() << endl;
    }

// *************PARAMETER***************
    std::string parameterName;
    std::string type;
    std::string value;

    // Retrieve all the parameters from the parameter table that are required for the experiment we're entering a run for
    result = conn.sql("SELECT * FROM ParameterTypes WHERE experimentId=? AND required=?").bind(experimentId).bind(true).execute();

    if (result.count() >= 1) {
        cout << endl << "Please enter data for all required parameters of experiment " << experimentId << endl;

        // Loop through every required parameter
        for (Row row : result.fetchAll()) {

            // We already can fetch the type and parameter name
            parameterName = row[1];
            type = row[2];

            // Enter a value for the parameter
            if (type.compare("INT") == 0) {
                do {
                    cout << endl;
                    cout << "Please enter a value for the parameter " << parameterName << endl;
                    cout << "You must enter an INT type" << endl;
                    cout << "Please use only numeric digits" << endl;
                    cout << ">> ";
                    getline(cin, value);
                } while (!regex_match(value, intValueR));
            }
            else if (type.compare("FLOAT") == 0) {
                do {
                    cout << endl;
                    cout << "Please enter a value for the parameter " << parameterName << endl;
                    cout << "You must enter an FLOAT type" << endl;
                    cout << "Please use only numeric digits with an optional decimal" << endl;
                    cout << ">> ";
                    getline(cin, value);
                } while (!regex_match(value, floatValueR));
            }
            else if (type.compare("STRING") == 0) {
                cout << endl;
                cout << "Please enter a value for the parameter " << parameterName << endl;
                cout << "You must enter a STRING type" << endl;
                cout << "You are free to use any characters you wish" << endl;
                cout << ">> ";
                getline(cin, value);
            }
            else if (type.compare("URL") == 0) {
                do {
                    cout << endl;
                    cout << "Please enter a value for the parameter " << parameterName << endl;
                    cout << "You must enter an URL type" << endl;
                    cout << "Please format your url in the format [a-zA-z].[a-zA-z].[a-zA-z]{3}" << endl;
                    cout << ">> ";
                    getline(cin, value);
                } while (!regex_match(value, urlValueR));
            }
            else if (type.compare("TIME") == 0) {
                do {
                    cout << "Please enter a value for the parameter " << parameterName << endl;
                    cout << "You must enter an TIME type" << endl;
                    cout << "Please format your time in the format HH:MM:SS" << endl;
                    cout << ">> ";
                    getline(cin, value);
                } while (!regex_match(value, timeValueR));
            }

            // Insert the parameter for the run
            try {
                // Build the SQL query
                cout << "Inserting run parameter " << parameterName << " - ";
                SqlStatement insertStatement = conn.sql(
                    "INSERT INTO RunsParameter " \
                    "VALUES(?,?,?,?)"
                );

                // Bind the values to the query
                insertStatement.bind(experimentId);
                insertStatement.bind(date + " " + time);
                insertStatement.bind(parameterName);
                insertStatement.bind(value);

                // Execute the query and store the result
                SqlResult resultCode = insertStatement.execute();

                // If there's no warnings then it was a success
                if (resultCode.getWarningsCount() <= 0) {
                    cout << "success." << endl;
                } // If there are warnings then print error message and quit
                else {
                    cout << "fail." << endl;
                    return;
                }
            }
            catch (const Error &err) {
                cout << err.what() << endl;
            }
        }
    }
    else {
        cout << "No required parameters" << endl;
    }

    // Check if there are any additional parameters, so they don't get stuck in a loop
    result = conn.sql("SELECT parameterName FROM ParameterTypes WHERE experimentId=? AND required=?").bind(experimentId).bind(false).execute();

    // If there are other possible parameters, ask the user if they want to insert them
    if (result.count() >= 1) {

        // Print out all the possible parameters
        cout << endl;
        cout << "+------------------------------------------------------------+" << endl;
        cout << "|                   Additional Parameters                    |" << endl;
        cout << "+------------------------------------------------------------+" << endl;
        for (Row row : result.fetchAll()) {
            cout << "| >> Parameter       - ";
            cout << setw(38) << left << row[0] << "|" << endl;
            cout << "+------------------------------------------------------------+" << endl;
        }

        // Loop until the user doesn't want to enter more parameters
        // Loop until there aren't any more parameters to enter
        int count = 0;
        char ch;
        do {
            cout << "Do you have any additional run parameters? (y/n) " << endl;
            cout << ">> ";
            cin >> ch;
            getline(cin, abandon);

            // If the user has more parameters
            if (ch == 'y' || ch == 'Y') {

                cout << "Are you sure?" << endl;
                cout << ">> ";
                cin >> ch;
                getline(cin, abandon);

                if (ch == 'y' || ch == 'Y') {

                    // Increment count to check against total number of possible parameters
                    count++;

                    std::string type;
                    do {
                        // Have the user enter the parameter name
                        cout << "Please enter a parameter name " << endl;
                        cout << ">> ";
                        getline(cin, parameterName);

                        // Check if the parameter is in the parameter table
                        result = conn.sql("SELECT * FROM ParameterTypes WHERE experimentId=? AND parameterName=?").bind(experimentId).bind(parameterName).execute();
                        errorCode = result.count();

                        if (errorCode <= 0) {
                            cout << "Please enter a valid parameter name" << endl;
                        }
                        else {
                            Row row = result.fetchOne();
                            type = row[2];
                        }

                    } while (errorCode <= 0);

                    // Enter a value for the parameter
                    if (type.compare("INT") == 0) {
                        do {
                            cout << endl;
                            cout << "Please enter a value for the parameter " << parameterName << endl;
                            cout << "You must enter an INT type" << endl;
                            cout << "Please use only numeric digits" << endl;
                            cout << ">> ";
                            getline(cin, value);
                        } while (!regex_match(value, intValueR));
                    }
                    else if (type.compare("FLOAT") == 0) {
                        do {
                            cout << endl;
                            cout << "Please enter a value for the parameter " << parameterName << endl;
                            cout << "You must enter an FLOAT type" << endl;
                            cout << "Please use only numeric digits with an optional decimal" << endl;
                            cout << ">> ";
                            getline(cin, value);
                        } while (!regex_match(value, floatValueR));
                    }
                    else if (type.compare("STRING") == 0) {
                        cout << endl;
                        cout << "Please enter a value for the parameter " << parameterName << endl;
                        cout << "You must enter a STRING type" << endl;
                        cout << "You are free to use any characters you wish" << endl;
                        cout << ">> ";
                        getline(cin, value);
                    }
                    else if (type.compare("URL") == 0) {
                        do {
                            cout << endl;
                            cout << "Please enter a value for the parameter " << parameterName << endl;
                            cout << "You must enter an URL type" << endl;
                            cout << "Please format your url in the format [a-zA-z].[a-zA-z].[a-zA-z]{3}" << endl;
                            cout << ">> ";
                            getline(cin, value);
                        } while (!regex_match(value, urlValueR));
                    }
                    else if (type.compare("TIME") == 0) {
                        do {
                            cout << endl;
                            cout << "Please enter a value for the parameter " << parameterName << endl;
                            cout << "You must enter an TIME type" << endl;
                            cout << "Please format your time in the format HH:MM:SS" << endl;
                            cout << ">> ";
                            getline(cin, value);
                        } while (!regex_match(value, timeValueR));
                    }

                    // Insert the parameter
                    try {
                        cout << "Inserting run parameter " << parameterName << " - ";
                        SqlStatement insertStatement = conn.sql(
                            "INSERT INTO RunsParameter " \
                            "VALUES(?,?,?,?)"
                        );
                        insertStatement.bind(experimentId);
                        insertStatement.bind(date + " " + time);
                        insertStatement.bind(parameterName);
                        insertStatement.bind(value);
                        SqlResult resultCode = insertStatement.execute();
                        if (resultCode.getWarningsCount() <= 0) {
                            cout << "success." << endl;
                        }
                        else {
                            cout << "fail." << endl;
                            return;
                        }
                    }
                    catch (const Error &err) {
                        cout << err.what() << endl;
                    }
                }
                else {
                    ch = ' ';
                }
            }
            else if (ch != 'n' && ch != 'N') {
                cout << "Unknown input" << endl;
            }


        } while (ch != 'n' && ch != 'N' && count < result.count());
    }

    // *************RESULT***************
    std::string resultName;

    // Retrieve all the parameters from the parameter table that are required for the experiment we're entering a run for
    result = conn.sql("SELECT * FROM ResultTypes WHERE experimentId=? AND required=?").bind(experimentId).bind(true).execute();

    if (result.count() >= 1) {
        cout << endl << "Please enter data for all required results of experiment " << experimentId << endl;
        // Loop through every required parameter
        for (Row row : result.fetchAll()) {

            // We already can fetch the type and parameter name
            resultName = row[1];
            type = row[2];

            // Enter a value for the parameter
            if (type.compare("INT") == 0) {
                do {
                    cout << endl;
                    cout << "Please enter a value for the result " << resultName << endl;
                    cout << "You must enter an INT type" << endl;
                    cout << "Please use only numeric digits" << endl;
                    cout << ">> ";
                    getline(cin, value);
                } while (!regex_match(value, intValueR));
            }
            else if (type.compare("FLOAT") == 0) {
                do {
                    cout << endl;
                    cout << "Please enter a value for the result " << resultName << endl;
                    cout << "You must enter an FLOAT type" << endl;
                    cout << "Please use only numeric digits with an optional decimal" << endl;
                    cout << ">> ";
                    getline(cin, value);
                } while (!regex_match(value, floatValueR));
            }
            else if (type.compare("STRING") == 0) {
                cout << endl;
                cout << "Please enter a value for the result " << resultName << endl;
                cout << "You must enter a STRING type" << endl;
                cout << "You are free to use any characters you wish" << endl;
                cout << ">> ";
                getline(cin, value);
            }
            else if (type.compare("URL") == 0) {
                do {
                    cout << endl;
                    cout << "Please enter a value for the result " << resultName << endl;
                    cout << "You must enter an URL type" << endl;
                    cout << "Please format your url in the format [a-zA-z].[a-zA-z].[a-zA-z]{3}" << endl;
                    cout << ">> ";
                    getline(cin, value);
                } while (!regex_match(value, urlValueR));
            }
            else if (type.compare("TIME") == 0) {
                do {
                    cout << endl;
                    cout << "Please enter a value for the result " << resultName << endl;
                    cout << "You must enter an TIME type" << endl;
                    cout << "Please format your time in the format HH:MM:SS" << endl;
                    cout << ">> ";
                    getline(cin, value);
                } while (!regex_match(value, timeValueR));
            }

            // Insert the parameter for the run
            try {
                cout << "Inserting run result " << resultName << " - ";
                SqlStatement insertStatement = conn.sql(
                    "INSERT INTO RunsResult " \
                    "VALUES(?,?,?,?)"
                );
                insertStatement.bind(experimentId);
                insertStatement.bind(date + " " + time);
                insertStatement.bind(resultName);
                insertStatement.bind(value);
                SqlResult resultCode = insertStatement.execute();
                if (resultCode.getWarningsCount() <= 0) {
                    cout << "success." << endl;
                }
                else {
                    cout << "fail." << endl;
                    return;
                }
            }
            catch (const Error &err) {
                cout << err.what() << endl;
            }
        }
    }
    else {
        cout << "No required parameters" << endl;
    }

    // Check if there are any additional parameters, so they don't get stuck in a loop
    result = conn.sql("SELECT resultName FROM ResultTypes WHERE experimentId=? AND required=?").bind(experimentId).bind(false).execute();
    if (result.count() >= 1) {

        // Print out all the possible results
        cout << endl;
        cout << "+------------------------------------------------------------+" << endl;
        cout << "|                    Additional Results                      |" << endl;
        cout << "+------------------------------------------------------------+" << endl;
        
        for (Row row : result.fetchAll()) {
            cout << "| >> Result          - ";
            cout << setw(38) << left << row[0] << "|" << endl;
            cout << "+------------------------------------------------------------+" << endl;
        }

        // Loop until the user doesn't want to enter more parameters
        // Loop until there aren't any more parameters to enter
        int count = 0;
        char ch;
        do {
            cout << "Do you have any additional run results? (y/n) " << endl;
            cout << ">> ";
            cin >> ch;
            getline(cin, abandon);

            // If the user has more parameters
            if (ch == 'y' || ch == 'Y') {

                cout << "Are you sure?" << endl;
                cout << ">> ";
                cin >> ch;
                getline(cin, abandon);

                if (ch == 'y' || ch == 'Y') {

                    // Increment count to check against total number of possible parameters
                    count++;

                    do {
                        // Have the user enter the parameter name
                        cout << "Please enter a result name " << endl;
                        cout << ">> ";
                        getline(cin, resultName);

                        // Check if the parameter is in the parameter table
                        result = conn.sql("SELECT * FROM ResultTypes WHERE experimentId=? AND resultName=?").bind(experimentId).bind(resultName).execute();
                        errorCode = result.count();

                        if (errorCode <= 0) {
                            cout << "Please enter a valid result name" << endl;
                        }
                        else {
                            Row row = result.fetchOne();
                            type = row[2];
                        }

                    } while (errorCode <= 0);

                    // Enter a value for the parameter
                    if (type.compare("INT") == 0) {
                        do {
                            cout << endl;
                            cout << "Please enter a value for the result " << resultName << endl;
                            cout << "You must enter an INT type" << endl;
                            cout << "Please use only numeric digits" << endl;
                            cout << ">> ";
                            getline(cin, value);
                        } while (!regex_match(value, intValueR));
                    }
                    else if (type.compare("FLOAT") == 0) {
                        do {
                            cout << endl;
                            cout << "Please enter a value for the result " << resultName << endl;
                            cout << "You must enter an FLOAT type" << endl;
                            cout << "Please use only numeric digits with an optional decimal" << endl;
                            cout << ">> ";
                            getline(cin, value);
                        } while (!regex_match(value, floatValueR));
                    }
                    else if (type.compare("STRING") == 0) {
                        cout << endl;
                        cout << "Please enter a value for the result " << resultName << endl;
                        cout << "You must enter a STRING type" << endl;
                        cout << "You are free to use any characters you wish" << endl;
                        cout << ">> ";
                        getline(cin, value);
                    }
                    else if (type.compare("URL") == 0) {
                        do {
                            cout << endl;
                            cout << "Please enter a value for the result " << resultName << endl;
                            cout << "You must enter an URL type" << endl;
                            cout << "Please format your url in the format [a-zA-z].[a-zA-z].[a-zA-z]{3}" << endl;
                            cout << ">> ";
                            getline(cin, value);
                        } while (!regex_match(value, urlValueR));
                    }
                    else if (type.compare("TIME") == 0) {
                        do {
                            cout << endl;
                            cout << "Please enter a value for the result " << resultName << endl;
                            cout << "You must enter an TIME type" << endl;
                            cout << "Please format your time in the format HH:MM:SS" << endl;
                            cout << ">> ";
                            getline(cin, value);
                        } while (!regex_match(value, timeValueR));
                    }

                    // Insert the parameter
                    try {
                        cout << "Inserting run result " << resultName << " - ";
                        SqlStatement insertStatement = conn.sql(
                            "INSERT INTO RunsResult " \
                            "VALUES(?,?,?,?)"
                        );
                        insertStatement.bind(experimentId);
                        insertStatement.bind(date + " " + time);
                        insertStatement.bind(resultName);
                        insertStatement.bind(value);
                        SqlResult resultCode = insertStatement.execute();
                        if (resultCode.getWarningsCount() <= 0) {
                            cout << "success." << endl;
                        }
                        else {
                            cout << "fail." << endl;
                            return;
                        }
                    }
                    catch (const Error &err) {
                        cout << err.what() << endl;
                    }
                }
                else {
                    ch = ' ';
                }
            }
            else if (ch != 'n' && ch != 'N') {
                cout << "Unknown input" << endl;
            }

        } while (ch != 'n' && ch != 'N' && count < result.count());
    }
}

void displayExperimentMetaData(Session& conn) {
    std::string abandon;

    std::string experimentId;
    int errorCode;

    // Loop until a valid experiment is chosen
    do {
        // Read in the experiment ID
        cout << "Please enter an experiment ID " << endl;
        cout << ">> ";
        getline(cin, experimentId);

        // Test if the experimentId exists in the Experiment table
        SqlResult result = conn.sql("SELECT * FROM Experiment WHERE experimentId=?").bind(experimentId).execute();
        errorCode = result.count();

        if (errorCode <= 0) {
            cout << "Please enter a valid experimentId" << endl;
        }

    } while (errorCode <= 0);

    cout << "+------------------------------------------------------------+" << endl;
    cout << "|                    Experiment MetaData                     |" << endl;
    cout << "+------------------------------------------------------------+" << endl;
    SqlResult result = conn.sql("SELECT ManagerID, FORMAT(StartDate, 'YYYY/MM/DD'), FORMAT(DataEntryDate, 'YYYY/MM/DD') FROM Experiment WHERE experimentId=?").bind(experimentId).execute();

    Row row;
    std::string managerId;
    std::string startDate;
    std::string dataEntryDate;

    while (row = result.fetchOne()) {

        managerId = row[0];
        startDate = row[1];
        dataEntryDate = row[2];

        startDate.erase(std::remove(startDate.begin(), startDate.end(), ','), startDate.end());
        dataEntryDate.erase(std::remove(dataEntryDate.begin(), dataEntryDate.end(), ','), dataEntryDate.end());

        cout << "| >> Manager Id      - ";
        cout << setw(38) << left << managerId << "|" << endl;
        cout << "| >> Start Date      - ";
        cout << setw(38) << left << startDate.substr(4, 2) + "-" + startDate.substr(6) + "-" + startDate.substr(0, 4) << "|" << endl;
        cout << "| >> Data Entry Date - ";
        cout << setw(38) << left << dataEntryDate.substr(4, 2) + "-" + dataEntryDate.substr(6) + "-" + dataEntryDate.substr(0, 4) << "|" << endl;
        cout << "+------------------------------------------------------------+" << endl;
    }

    cout << endl;
    cout << "+------------------------------------------------------------+" << endl;
    cout << "|                    Parameter MetaData                      |" << endl;
    cout << "+------------------------------------------------------------+" << endl;
    result = conn.sql("SELECT * FROM ParameterTypes WHERE experimentId=?").bind(experimentId).execute();

    std::string parameterName;
    std::string type;
    bool required;
    while (row = result.fetchOne()) {

        parameterName = row[1];
        type = row[2];
        required = row[3];

        cout << "| >> Parameter Name  - ";
        cout << setw(38) << left << parameterName << "|" << endl;
        cout << "| >> Type            - ";
        cout << setw(38) << left << type << "|" << endl;
        cout << "| >> Required        - ";
        cout << setw(38) << left << (required ? "Yes" : "No") << "|" << endl;
        cout << "+------------------------------------------------------------+" << endl;
    }

    cout << endl;
    cout << "+------------------------------------------------------------+" << endl;
    cout << "|                    Result MetaData                         |" << endl;
    cout << "+------------------------------------------------------------+" << endl;
    result = conn.sql("SELECT * FROM ResultTypes WHERE experimentId=?").bind(experimentId).execute();

    std::string resultName;
    while (row = result.fetchOne()) {

        resultName = row[1];
        type = row[2];
        required = row[3];

        cout << "| >> Result Name     - ";
        cout << setw(38) << left << resultName << "|" << endl;
        cout << "| >> Type            - ";
        cout << setw(38) << left << type << "|" << endl;
        cout << "| >> Required        - ";
        cout << setw(38) << left << (required ? "Yes" : "No") << "|" << endl;
        cout << "+------------------------------------------------------------+" << endl;
    }
}

void displayRunMetaData(Session& conn) {
    std::string abandon;

    std::string experimentId;
    int errorCode;

    // Loop until a valid experiment is chosen
    do {
        // Read in the experiment ID
        cout << "Please enter an experiment ID " << endl;
        cout << ">> ";
        getline(cin, experimentId);

        // Test if the experimentId exists in the Experiment table
        SqlResult result = conn.sql("SELECT * FROM Experiment WHERE experimentId=?").bind(experimentId).execute();
        errorCode = result.count();

        if (errorCode <= 0) {
            cout << "Please enter a valid experimentId" << endl;
        }

    } while (errorCode <= 0);

    SqlResult result = conn.sql("SELECT FORMAT(TimeOfRun, 'YYYY/MM/DD'), ExperimenterSSN, Success FROM Runs WHERE experimentId=?").bind(experimentId).execute();

    if (result.count() <= 0) {
        cout << "No runs detected for this experiment" << endl;
        return;
    }

    cout << "+------------------------------------------------------------+" << endl;
    cout << "|                       Possible Runs                        |" << endl;
    cout << "+------------------------------------------------------------+" << endl;
    int count = 0;

    for (Row row : result.fetchAll()) {
        std::string timeOfRun = row[0];
        timeOfRun.erase(std::remove(timeOfRun.begin(), timeOfRun.end(), ','), timeOfRun.end());
        std::string formattedTimeOfRun = timeOfRun.substr(0, 4) + "-" + timeOfRun.substr(4, 2) + "-" + timeOfRun.substr(6, 2) + " " + timeOfRun.substr(8, 2) + ":" + timeOfRun.substr(10, 2) + ":" + timeOfRun.substr(12, 2);

        cout << "| " << setw(3) << left << count << " >> ";
        cout << setw(52) << left << formattedTimeOfRun << "|" << endl;
        cout << "+------------------------------------------------------------+" << endl;
        count++;
    }

    int index;
    do {
        cout << "Please enter the index of the run" << endl;
        cout << ">> ";
        cin >> index;
    } while (index < 0 || index >= count);

    Row row;
    result = conn.sql("SELECT FORMAT(TimeOfRun, 'YYYY/MM/DD'), ExperimenterSSN, Success FROM Runs WHERE experimentId=?").bind(experimentId).execute();
    for (int i = 0; i <= index; i++) {
        row = result.fetchOne();
    }
    
    std::string timeOfRun;
    std::string formattedTimeOfRun;

    cout << "+------------------------------------------------------------+" << endl;
    cout << "|                      Run MetaData                          |" << endl;
    cout << "+------------------------------------------------------------+" << endl;

    timeOfRun = row[0];
    timeOfRun.erase(std::remove(timeOfRun.begin(), timeOfRun.end(), ','), timeOfRun.end());
    formattedTimeOfRun = timeOfRun.substr(0, 4) + "-" + timeOfRun.substr(4, 2) + "-" + timeOfRun.substr(6, 2) + " " + timeOfRun.substr(8,2) + ":" + timeOfRun.substr(10, 2) + ":" + timeOfRun.substr(12, 2);
        
    cout << "| >> Time of Run     - ";
    cout << setw(38) << left << formattedTimeOfRun << "|" << endl;
    cout << "| >> Experimenter ID - ";
    cout << setw(38) << left << row[1] << "|" << endl;
    cout << "| >> Success         - ";
    cout << setw(38) << left << (row[2] ? "Yes" : "No") << "|" << endl;
    cout << "+------------------------------------------------------------+" << endl;

    cout << endl;
    cout << "+------------------------------------------------------------+" << endl;
    cout << "|                   Parameter MetaData                       |" << endl;
    cout << "+------------------------------------------------------------+" << endl;

    SqlResult parameterResult = conn.sql("SELECT parameterName, value FROM RunsParameter WHERE experimentId=? AND timeOfRun=?").bind(experimentId).bind(formattedTimeOfRun).execute();

    Row parameterRow;
    while (parameterRow = parameterResult.fetchOne()) {
        cout << "| >> Parameter Name  - ";
        cout << setw(38) << left << parameterRow[0] << "|" << endl;
        cout << "| >> Value           - ";
        cout << setw(38) << left << parameterRow[1] << "|" << endl;
        cout << "+------------------------------------------------------------+" << endl;
    }

    cout << endl;
    cout << "+------------------------------------------------------------+" << endl;
    cout << "|                     Result MetaData                        |" << endl;
    cout << "+------------------------------------------------------------+" << endl;

    SqlResult resultResult = conn.sql("SELECT resultName, value FROM RunsResult WHERE experimentId=? AND TimeOfRun=?").bind(experimentId).bind(formattedTimeOfRun).execute();
        
    Row resultRow;
    while (resultRow = resultResult.fetchOne()) {
        cout << "| >> Result Name     - ";
        cout << setw(38) << left << resultRow[0] << "|" << endl;
        cout << "| >> Value           - ";
        cout << setw(38) << left << resultRow[1] << "|" << endl;
        cout << "+------------------------------------------------------------+" << endl;
    }
    
}

void generateExperimentReport(Session& conn) {
    std::string experimentId;
    int errorCode;

    // Loop until a valid experiment is chosen
    do {
        // Read in the experiment ID
        cout << "Please enter an experiment ID " << endl;
        cout << ">> ";
        getline(cin, experimentId);

        // Test if the experimentId exists in the Experiment table
        SqlResult result = conn.sql("SELECT * FROM Experiment WHERE experimentId=?").bind(experimentId).execute();
        errorCode = result.count();

        if (errorCode <= 0) {
            cout << "Please enter a valid experimentId" << endl;
        }

    } while (errorCode <= 0);

    // Open the file
    ofstream report;
    report.open("experimentReport.html");

    report << "<html>" << endl;
    report << "<head>" << endl;
    report << "<style>" << endl;
    report << "table{border-collapse: collapse;}" << endl;
    report << "table, th, td{border: 1px solid black; }" << endl;
    report << ".inline{display: inline-block;}" << endl;
    report << ".inline table{float: left; margin: 5px;}" << endl;
    report << "</style>" << endl;
    report << "</head>" << endl;
    report << "<body>" << endl;

    SqlResult result = conn.sql("SELECT FORMAT(TimeOfRun, 'YYYY/MM/DD'), ExperimenterSSN, Success FROM Runs WHERE experimentId=?").bind(experimentId).execute();

    for (Row runRow : result.fetchAll()) {
        std::string timeOfRun;
        std::string experimenterId;
        bool success ;

        timeOfRun = runRow[0];
        experimenterId = runRow[1];
        success = runRow[2];
        timeOfRun.erase(std::remove(timeOfRun.begin(), timeOfRun.end(), ','), timeOfRun.end());
        std::string formattedTimeOfRun = timeOfRun.substr(0, 4) + "-" + timeOfRun.substr(4, 2) + "-" + timeOfRun.substr(6, 2) + " " + timeOfRun.substr(8, 2) + ":" + timeOfRun.substr(10, 2) + ":" + timeOfRun.substr(12, 2);

        report << "</br>" << endl;
        report << "<div class=\"inline\">" << endl;
        report << "<table>" << endl;
        report << "<col width=\"200\">" << endl;
        report << "<col width=\"200\">" << endl;
        report << "<tr>" << endl;
        report << "<td>Time of Run</td>" << endl;
        report << "<td>" << formattedTimeOfRun << "</td>" << endl;
        report << "</tr>" << endl;

        report << "<tr>" << endl;
        report << "<td>Experimenter ID</td>" << endl;
        report << "<td>" << experimenterId << "</td>" << endl;
        report << "</tr>" << endl;

        report << "<tr>" << endl;
        report << "<td>Success</td>" << endl;
        report << "<td>" << (success ? "Yes" : "No") << "</td>" << endl;
        report << "</tr>" << endl;
        report << "</table>" << endl;
        report << "</div>" << endl;
        SqlResult parameterResult = conn.sql("SELECT ParameterName, Value FROM RunsParameter WHERE experimentId=? AND TimeOfRun=?").bind(experimentId).bind(formattedTimeOfRun).execute();

        report << "</br>" << endl;
        report << "<div class=\"inline\">" << endl;
        for (Row parameterRow : parameterResult.fetchAll()) {
            std::string parameterName;
            std::string value;

            parameterName = parameterRow[0];
            value = parameterRow[1];

            report << "<table>" << endl;
            report << "<col width=\"200\">" << endl;
            report << "<col width=\"200\">" << endl;
            report << "<tr>" << endl;
            report << "<td>Parameter Name</td>" << endl;
            report << "<td>" << parameterName << "</td>" << endl;
            report << "</tr>" << endl;

            report << "<tr>" << endl;
            report << "<td>Value</td>" << endl;
            report << "<td>" << value << "</td>" << endl;
            report << "</tr>" << endl;
            report << "</table>" << endl;
        }
        report << "</div>" << endl;

        SqlResult resultResult = conn.sql("SELECT ResultName, Value FROM RunsResult WHERE experimentId=? AND TimeOfRun=?").bind(experimentId).bind(formattedTimeOfRun).execute();

        report << "</br>" << endl;
        report << "<div class=\"inline\">" << endl;
        for (Row resultRow : resultResult.fetchAll()) {
            std::string resultName;
            std::string value;

            resultName = resultRow[0];
            value = resultRow[1];

            report << "<table>" << endl;
            report << "<col width=\"200\">" << endl;
            report << "<col width=\"200\">" << endl;
            report << "<tr>" << endl;
            report << "<td>Result Name</td>" << endl;
            report << "<td>" << resultName << "</td>" << endl;
            report << "</tr>" << endl;

            report << "<tr>" << endl;
            report << "<td>Value</td>" << endl;
            report << "<td>" << value << "</td>" << endl;
            report << "</tr>" << endl;
            report << "</table>" << endl;
        }
        report << "</div>" << endl;

        report << "</br></br>" << endl;
    }

    report << "</body>" << endl;
    report << "</html>" << endl;
}

void generateAggregateReport(Session& conn) {
    std::string abandon;
    std::string experimentId;
    int errorCode;

    // Loop until a valid experiment is chosen
    do {
        // Read in the experiment ID
        cout << "Please enter an experiment ID " << endl;
        cout << ">> ";
        getline(cin, experimentId);

        // Test if the experimentId exists in the Experiment table
        SqlResult result = conn.sql("SELECT * FROM Experiment WHERE experimentId=?").bind(experimentId).execute();
        errorCode = result.count();

        if (errorCode <= 0) {
            cout << "Please enter a valid experimentId" << endl;
        }

    } while (errorCode <= 0);

    SqlResult result = conn.sql("SELECT resultName, type, required FROM ResultTypes WHERE experimentId=? AND type=\"INT\" OR type=\"FLOAT\"").bind(experimentId).execute();

    if (result.count() <= 0) {
        cout << "No results eligible for aggregation" << endl;
        return;
    }

    cout << endl;
    cout << "+------------------------------------------------------------+" << endl;
    cout << "|                      Possible Results                      |" << endl;
    cout << "+------------------------------------------------------------+" << endl;
    
    int count = 0;
    for(Row row : result.fetchAll()) {
        cout << "| " << setw(3) << left << count << " >> ";
        cout << setw(52) << left << row[0] << "|" << endl;
        cout << "+------------------------------------------------------------+" << endl;
        count++;
    }

    int index;
    do {
        cout << "Please enter the index of the result" << endl;
        cout << ">> ";
        cin >> index;
    } while (index < 0 || index >= count);

    getline(cin, abandon);

    result = conn.sql("SELECT resultName, type, required FROM ResultTypes WHERE experimentId=? AND type=\"INT\" OR type=\"FLOAT\"").bind(experimentId).execute();
    Row row;
    for (int i = 0; i <= index; i++) {
        row = result.fetchOne();
    }

    std::string resultName = row[0];
    std::string type = row[1];
    bool required = row[2];

    regex dateR("^[0-9]{4}-[0-9]{1,2}-[0-9]{1,2}$");

    std::string beginningDate;
    do {
        cout << "Please enter the lower bound date (YYYY-MM-DD)" << endl;
        cout << ">> ";
        getline(cin, beginningDate);
    } while (!regex_match(beginningDate, dateR));

    std::string endDate;
    do {
        cout << "Please enter the upper bound date (YYYY-MM-DD)" << endl;
        cout << ">> ";
        getline(cin, endDate);
    } while (!regex_match(endDate, dateR) || (beginningDate.compare(endDate) == 0));

    result = conn.sql("SELECT ResultName, SUM(value), AVG(value) " \
                      "FROM RunsResult WHERE experimentId=?" \
                      "AND resultName=?" \
                      "AND TimeOfRun >= ?"
                      "AND TimeOfRun <= ?")
                    .bind(experimentId)
                    .bind(resultName)
                    .bind(beginningDate)
                    .bind(endDate)
                    .execute();

    if (result.count() <= 0) {
        cout << "Unable to aggregate result" << endl;
        return;
    }

    cout << endl;
    cout << "+------------------------------------------------------------+" << endl;
    cout << "|                          Results                           |" << endl;
    cout << "+------------------------------------------------------------+" << endl;
    while (row = result.fetchOne()) {

        cout << "| >> Result Name     - ";
        cout << setw(38) << left << row[0] << "|" << endl; 
        cout << "| >> Avg value       - ";
        cout << setw(38) << left << row[1] << "|" << endl;
        cout << "| >> Sum             - ";
        cout << setw(38) << left << row[2] << "|" << endl;
        cout << "+------------------------------------------------------------+" << endl;
    }

}

void parameterSearch(Session& conn) {
    std::string abandon;
    
    std::string parameterName;
    std::string type;

    cout << "Please enter a parameter name " << endl;
    cout << ">> ";
    getline(cin, parameterName);

    do {
        cout << "Please enter a parameter type" << endl;
        cout << ">> ";
        getline(cin, type);
    } while (type.compare("INT") != 0 &&
        type.compare("FLOAT") != 0 &&
        type.compare("STRING") != 0 &&
        type.compare("URL") != 0 &&
        type.compare("DATE") != 0 &&
        type.compare("DATETIME") != 0);

    SqlResult result = conn.sql("SELECT E.experimentId FROM Experiment E, ParameterTypes P " \
                                "WHERE E.experimentId = P.experimentId " \
                                "AND P.parameterName=? " \
                                "AND P.type=? " \
                                "ORDER BY E.startDate")
                                .bind(parameterName)
                                .bind(type)
                                .execute();

    if (result.count() <= 0) {
        cout << "No experiments found with parameter " << parameterName << " of type " << type << endl;
        return;
    }
    cout << endl;
    cout << "+------------------------------------------------------------+" << endl;
    cout << "|                       Experiments                          |" << endl;
    cout << "+------------------------------------------------------------+" << endl;

    Row row;
    while (row = result.fetchOne()) {
        cout << "| >> Experiment      - ";
        cout << setw(38) << left << row[0] << "|" << endl;
        cout << "+------------------------------------------------------------+" << endl;
    }
}