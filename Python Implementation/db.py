import json

JSON_FILENAME = "student_info.json"

def main():
	get_data()
	add_student("S5", "caleb", "pass3")
	get_data()

def get_data():

	with open(JSON_FILENAME, "r") as file:
		json_data = json.load(file)

		for i in json_data:
			print(i, json_data[i])

	return json_data


def add_student(student_id, name, password):
	new_student = {
		student_id : {
			"name": name,
			"password": password
		}
	}



	new_student_json = json.dumps(new_data)
	old_json_data = get_data()

	old_json_data.update(new_data)

	print(old_json_data)

	with open(JSON_FILENAME, "w") as file2:
		file2.write(json.dumps(old_json_data))


if __name__ == "__main__":
    main()
