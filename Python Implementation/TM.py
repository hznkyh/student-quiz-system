import json
import placeholderQB as QB


JSON_FILENAME = "student_info.json"




def main():

    with open(JSON_FILENAME, "r") as file:
        json_data = json.load(file)

        for i in json_data:
            print(i, json_data[i])


def get_question():
    # gets a question to give to 
    pass

def mark_question(question):
    pass



if __name__ == "__main__":

    # create QB object

    QB1 = QB.QB()

    main()


