import json

JSON_FILENAME = "student_info.json"

def read_records():
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
def print_student_summary():
    with open(JSON_FILENAME, "r") as file:
        json_data = json.load(file)

        print("Student Summary")
        for student_id, student in json_data.items():
            print(student_id, "->", student)


# checks if students login details are correct
def check_login(student_id, password):
    json_data = read_records()

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
def get_student(student_id):

    json_data = read_records()

    name = json_data[student_id]["name"]
    grade = json_data[student_id]["grade"]

    return {
        "name": name,
        "grade": grade
    }


# returns student grade
def get_grade(student_id):
    json_data = read_records()

    return json_data[student_id]["grade"]


# sets the grade of the student
def set_grade(student_id, new_grade):
    json_data = read_records()
    json_data[student_id]["grade"] = new_grade
    json_object = json.dumps(json_data, indent=4)

    with open(JSON_FILENAME, "w") as file:
        file.write(json_object)

def get_test_data(student_id):
    return read_records()[student_id]["test"]

def set_test_data(student_id, test_data):
    json_data = read_records()
    json_data[student_id]["test"] = test_data
    json_object = json.dumps(json_data, indent=4)

    with open(JSON_FILENAME, "w") as file:
        file.write(json_object)

def remaining_attempts(student_id, question_num):
    json_data = read_records()
    number = str(question_num)
    return json_data[student_id]["test"][number]["remaining_attempts"]

def set_remaining_attempts(student_id, question_num, remaining_attempts):
    json_data = read_records()
    number = str(question_num)
    json_data[student_id]["test"][number]["remaining_attempts"] = remaining_attempts
    json_object = json.dumps(json_data, indent=4)

    with open(JSON_FILENAME, "w") as file:
        file.write(json_object)

def get_active_state(student_id):
    json_data = read_records()
    if json_data[student_id]["active_test"]:
        return True
    else:
        return False

def set_test_active_state(student_id, active=True):
    json_data = read_records()
    json_data[student_id]["active_test"] = active
    json_object = json.dumps(json_data, indent=4)

    with open(JSON_FILENAME, "w") as file:
        file.write(json_object)



# resets all grades
def reset_grades():
    json_data = read_records()

    for student_id in json_data:
        set_grade(student_id, 0)
