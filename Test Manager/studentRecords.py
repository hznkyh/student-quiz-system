import json

JSON_FILENAME = "student_info.json"

def readRecords():
    """
    returns json records as a dict
    key is the student id (as a string)
    each key refers to as a sub dictionary with keys:
      name (str), grade(int), password(str)
    """
    with open(JSON_FILENAME, "r") as file:
        data = json.load(file)

        return data


# Probably not really necessary but gives a good basis for how to manage json files
def printStudentSummary():
    with open(JSON_FILENAME, "r") as file:
        json_data = json.load(file)

        print("Student Summary")
        for student_id, student in json_data.items():
            print(student_id, "->", student)


# checks if students login details are correct
def checkLogin(student_id, password):
    return True # temporary because ceebs logging in to test
    json_data = readRecords()

    if student_id in json_data:
        if json_data[student_id]["password"] == password:
            print("Correct login details!")
            print("Logged in as {}".format(json_data[student_id]["name"]))
            return True
        else:
            print("Incorrect login details.")
    else:
        print("Student id not on record")

    return False


# returns info on student in the form of a dictionary
def getStudent(student_id):

    json_data = readRecords()

    name = json_data[student_id]["name"]
    grade = json_data[student_id]["grade"]

    return {
        "name": name,
        "grade": grade
    }


# returns student grade
def getGrade(student_id):
    json_data = readRecords()

    return json_data[student_id]["grade"]


# sets the grade of the student
def setGrade(student_id, new_grade):
    json_data = readRecords()
    json_data[student_id]["grade"] = new_grade
    json_object = json.dumps(json_data, indent=4)

    with open(JSON_FILENAME, "w") as file:
        file.write(json_object)

def getTestData(student_id):
    return readRecords()[student_id]["test"]

def setTestData(student_id, test_data):
    json_data = readRecords()
    json_data[student_id]["test"] = test_data
    json_object = json.dumps(json_data, indent=4)

    with open(JSON_FILENAME, "w") as file:
        file.write(json_object)


# resets all grades
def resetGrades():
    json_data = readRecords()

    for student_id in json_data:
        setGrade(student_id, 0)
