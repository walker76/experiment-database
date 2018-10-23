#include "stdafx.h"
#include <iostream>
#include <mysqlx/xdevapi.h>
#include <string>
#include <ctime>

using namespace std;
using namespace mysqlx;

int initializeSession(Session& conn);
void insertExperiment(Session& conn);
void insertRun(Session& conn);
void displayExperimentMetaData(Session& conn);
void displayRunMetaData(Session& conn);

int main(int argc, char* argv[]) {
    try {
        SessionSettings initialOptions(
            SessionOption::USER, "root",
            SessionOption::PWD, "ALW1mys1@%",
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
                    cout << "4" << endl;
                    break;
                case '5':
                    cout << "5" << endl;
                    break;
                case '6':
                    cout << "6" << endl;
                    break;
                case 'q':
                    cout << "q" << endl;
                    return 0;
                    break;
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

    cout << "Please enter an experiment ID " << endl;
    cout << ">> ";
    getline(cin, experimentId);
    cout << experimentId << endl;
    
    do {
        cout << "Please enter a manager ID " << endl;
        cout << ">> ";
        getline(cin, managerId);

        if (managerId.size() > 6) {
            cout << "The manager ID must be no greater than 6 characters" << endl;
        }

    } while (managerId.size() > 6);

    //TODO - Regex for date
    cout << "Please enter a start date for the experiment (YYYY-MM-DD)" << endl;
    cout << ">> ";
    getline(cin, startDate);

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
                     type.compare("DATETIME") != 0 );
            
            do {
                cout << "Is this parameter required? (y/n)" << endl;
                cin >> chR;
                getline(cin, abandon);

                if (chR == 'y' || chR == 'Y') {
                    required = true;
                } else if (chR == 'n' || chR == 'N') {
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
        else if (ch != 'n' && ch != 'N') {
            cout << "Unknown input" << endl;
        }

    } while (ch != 'n' && ch != 'N');

    // *************RESULT***************
    std::string resultName;

    do {
        cout << "Do you have any result types? (y/n) " << endl;
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
        else if (ch != 'n' && ch != 'N') {
            cout << "Unknown input" << endl;
        }

    } while (ch != 'n' && ch != 'N');

}

void insertRun(Session& conn) {
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

    // Enter a date for the run
    // TODO - Regex for date
    std::string date;
    cout << "Please enter the date of the run (YYYY-MM-DD)" << endl;
    cout << ">> ";
    getline(cin, date);

    // Enter a time for the run
    // TODO - Regex for time
    std::string time;
    cout << "Please enter the time of the run (HH:MM:SS)" << endl;
    cout << ">> ";
    getline(cin, time);

    // Loop until a valid experimenter ID is given (< 6 characters)
    std::string experimenterId;
    do {
        cout << "Please enter the experimenter's ID" << endl;
        cout << ">> ";
        getline(cin, experimenterId);

        if (experimenterId.size() > 6) {
            cout << "The manager ID must be no greater than 6 characters" << endl;
        }

    } while (experimenterId.size() > 6);

    // loop until a yes or no is given
    char ch;
    char chS;
    bool success;
    do {
        cout << "Was this run a success? (y/n)" << endl;
        cin >> chS;
        getline(cin, abandon);

        if (chS == 'y' || chS == 'Y') {
            success = true;
        }
        else if (chS == 'n' || chS == 'N') {
            success = false;
        }
        else {
            cout << "Unknown input" << endl;
        }

    } while (chS != 'y' && chS != 'Y' && chS != 'n' && chS != 'N');

    // Insert the run
    try {
        cout << "Inserting run...";
        SqlStatement insertStatement = conn.sql(
            "INSERT INTO Runs " \
            "VALUES(?,?,?,?)"
        );
        insertStatement.bind(experimentId);
        insertStatement.bind(date + " " + time);
        insertStatement.bind(experimenterId);
        insertStatement.bind(success);
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
    std::string value;

    // Retrieve all the parameters from the parameter table that are required for the experiment we're entering a run for
    cout << "Please enter data for all required parameters of experiment " << experimentId << endl;
    SqlResult result = conn.sql("SELECT * FROM ParameterTypes WHERE experimentId=? AND required=?").bind(experimentId).bind(true).execute();

    // Loop through every required parameter
    Row row;
    while (row = result.fetchOne()) {

        // We already can fetch the type and parameter name
        parameterName = row[1];
        type = row[2];
        
        // Enter a value for the parameter
        // TODO - Regex for parameter type
        cout << "Please enter a value for the parameter " << parameterName << endl;
        getline(cin, value);

        // Insert the parameter for the run
        try {
            cout << "Inserting run parameter " << parameterName << " - ";
            SqlStatement insertStatement = conn.sql(
                "INSERT INTO RunsParamter " \
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

    // Check if there are any additional parameters, so they don't get stuck in a loop
    result = conn.sql("SELECT * FROM ParameterTypes WHERE experimentId=? AND required=?").bind(experimentId).bind(false).execute();
    if (result.count() >= 1) {

        // Loop until the user doesn't want to enter more parameters
        // Loop until there aren't any more parameters to enter
        int count = 0;
        do {
            cout << "Do you have any additional run parameters? (y/n) " << endl;
            cin >> ch;
            getline(cin, abandon);

            // If the user has more parameters
            if (ch == 'y' || ch == 'Y') {

                // Increment count to check against total number of possible parameters
                count++;

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

                } while (errorCode <= 0);

                // Enter a value for the parameter
                // TODO - Regex for the value's type
                cout << "Please enter a value for the parameter " << parameterName << endl;
                getline(cin, value);

                // Insert the parameter
                try {
                    cout << "Inserting run parameter " << parameterName << " - ";
                    SqlStatement insertStatement = conn.sql(
                        "INSERT INTO RunsParamter " \
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
            else if (ch != 'n' && ch != 'N') {
                cout << "Unknown input" << endl;
            }

        } while (ch != 'n' && ch != 'N' && count < result.count());
    }

    // *************RESULT***************
    std::string resultName;

    // Retrieve all the parameters from the parameter table that are required for the experiment we're entering a run for
    cout << "Please enter data for all required results of experiment " << experimentId << endl;
    result = conn.sql("SELECT * FROM ResultTypes WHERE experimentId=? AND required=?").bind(experimentId).bind(true).execute();

    // Loop through every required parameter
    while (row = result.fetchOne()) {

        // We already can fetch the type and parameter name
        resultName = row[1];
        type = row[2];

        // Enter a value for the parameter
        // TODO - Regex for parameter type
        cout << "Please enter a value for the result " << resultName << endl;
        getline(cin, value);

        // Insert the parameter for the run
        try {
            cout << "Inserting run parameter " << resultName << " - ";
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

    // Check if there are any additional parameters, so they don't get stuck in a loop
    result = conn.sql("SELECT * FROM ResultTypes WHERE experimentId=? AND required=?").bind(experimentId).bind(false).execute();
    if (result.count() >= 1) {

        // Loop until the user doesn't want to enter more parameters
        // Loop until there aren't any more parameters to enter
        int count = 0;
        do {
            cout << "Do you have any additional run results? (y/n) " << endl;
            cin >> ch;
            getline(cin, abandon);

            // If the user has more parameters
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
                        cout << "Please enter a valid parameter name" << endl;
                    }

                } while (errorCode <= 0);

                // Enter a value for the parameter
                // TODO - Regex for the value's type
                cout << "Please enter a value for the result " << resultName << endl;
                getline(cin, value);

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

    cout << "Experiment MetaData:" << endl;
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

        cout << ">> Manager Id - " << managerId << endl;
        cout << ">> Start Date - " << startDate.substr(4,2) << "/" << startDate.substr(6) << "/" << startDate.substr(0, 4) << endl;
        cout << ">> Data Entry Date - " << dataEntryDate.substr(4,2) << "/" << dataEntryDate.substr(6) << "/" << dataEntryDate.substr(0, 4) << endl;
    }

    cout << "\nParameter MetaData:" << endl;
    result = conn.sql("SELECT * FROM ParameterTypes WHERE experimentId=?").bind(experimentId).execute();

    std::string parameterName;
    std::string type;
    bool required;
    while (row = result.fetchOne()) {

        parameterName = row[1];
        type = row[2];
        required = row[3];

        cout << ">> Parameter Name - " << parameterName << endl;
        cout << ">> Type - " << type << endl;
        cout << ">> Required - " << (required ? "Yes" : "No") << endl << endl;
    }

    cout << "\nResult MetaData:" << endl;
    result = conn.sql("SELECT * FROM ResultTypes WHERE experimentId=?").bind(experimentId).execute();

    std::string resultName;
    while (row = result.fetchOne()) {

        resultName = row[1];
        type = row[2];
        required = row[3];

        cout << ">> Result Name - " << resultName << endl;
        cout << ">> Type - " << type << endl;
        cout << ">> Required - " << (required ? "Yes" : "No") << endl << endl;
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

    SqlResult result = conn.sql("SELECT FORMAT(TimeOfRun, 'YYYY/MM/DD'), ExperimenterSSN, Success, TimeOfRun FROM Runs WHERE experimentId=?").bind(experimentId).execute();

    Row row;
    std::string timeOfRun;
    std::string experimenterSSN;
    bool success;

    while (row = result.fetchOne()) {

        cout << "Run MetaData:" << endl;

        timeOfRun = row[0];
        experimenterSSN = row[1];
        success = row[2];

        timeOfRun.erase(std::remove(timeOfRun.begin(), timeOfRun.end(), ','), timeOfRun.end());

        cout << ">> Time of Run - " << timeOfRun.substr(4, 2) << "/" << timeOfRun.substr(6) << "/" << timeOfRun.substr(0, 4) << endl;
        cout << ">> Experiment ID - " << experimenterSSN << endl;
        cout << ">> Success - " << (success ? "Yes" : "No") << endl << endl;

        cout << "\nParameter MetaData:" << endl;
        SqlResult parameterResult = conn.sql("SELECT parameterName, value FROM RunsParameter WHERE experimentId=? AND TimeOfRun=?").bind(experimentId).bind(row[3]).execute();

        std::string parameterName;
        std::string value;

        Row parameterRow;
        while (parameterRow = parameterResult.fetchOne()) {

            parameterName = parameterRow[1];
            value = parameterRow[2];

            cout << ">> Experiment ID - " << experimentId << endl;
            cout << ">> Time of Run - " << timeOfRun.substr(4, 2) << "/" << timeOfRun.substr(6) << "/" << timeOfRun.substr(0, 4) << endl;
            cout << ">> Parameter Name - " << parameterName << endl;
            cout << ">> Value - " << value << endl;
        }

        cout << "\nResult MetaData:" << endl;
        SqlResult resultResult = conn.sql("SELECT resultName, value FROM RunsResult WHERE experimentId=? AND TimeOfRun=?").bind(experimentId).bind(row[3]).execute();

        std::string resultName;

        Row resultRow;
        while (resultRow = resultResult.fetchOne()) {

            resultName = resultRow[1];
            value = resultRow[2];

            cout << ">> Result Name - " << resultName << endl;
            cout << ">> Type - " << type << endl;
            cout << ">> Required - " << (required ? "Yes" : "No") << endl << endl;
        }
    }
}