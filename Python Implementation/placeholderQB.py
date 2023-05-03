# this is a placeholder QB object
# will imitate the actual implementation of a QB
import csv
import random

QUESTION_FILENAME = "questions.csv"


class QB:
    def __init__(self):
        print("instantiated a QB object {}".format(id(self)))

        self.question_bank = {}
        self.num_questions = 0

        # initiate questions
        with open(QUESTION_FILENAME, newline='') as csvfile:
            reader = csv.reader(csvfile, delimiter=',')

            id_count = 0
            for row in reader:
                question = row[0]
                options = row[1:5]
                answer = row[5]

                self.question_bank[id_count] = {
                    "question": question,
                    "options": options,
                    "answer": answer
                }

                id_count += 1

            self.num_questions = id_count
            if self.num_questions == 0:
                raise Exception("Warning, zero questions")

    def sendQuestion(self):
        rand_id = random.randint(0, self.num_questions - 1)

        return {
            "id": rand_id,
            "question": self.question_bank[rand_id]["question"],
            "options": self.question_bank[rand_id]["options"]
        }

    def markQuestion(self, question_id, answer):
        try:
            if answer == self.question_bank[question_id][answer]:
                return True
        except KeyError:
            return False


if __name__ == "__main__":
    qb1 = QB()

    print(qb1.sendQuestion())
    print(qb1.markQuestion(0, "Seattle"))











