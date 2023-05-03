import json
import random
import placeholderQB as QB

JSON_FILENAME = "student_info.json"


def main():
    # main loop
    # initialise html interface
    # wait for response
    printStudentSummary()


class Test:
    session_ids = []

    def __init__(self, num_questions):
        self.questions = []
        self.session_id

        # create a session id to keep track of the test
        if len(session_ids) == 0:
            self.session_id = 0
        else:
            self.session_id = session_ids[-1] +1


def printStudentSummary():
    with open(JSON_FILENAME, "r") as file:
        json_data = json.load(file)

        print("Student Summary")
        for student_id, student in json_data.items():
            print(student_id, "->", student)


def get_question(question_bank):
    question = question_bank.get_question()
    return question
    
    # display question to 


def mark_question(question_bank, answer):
    correct = question_bank.mark_question()

    if correct:
        # adjust mark as necessary

        pass



if __name__ == "__main__":
    # create QB object

    QB1 = QB.QB()

    main()
