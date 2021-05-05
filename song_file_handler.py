
ESP_SONG_FILE_PATH = "/var/jail/home/gagordon/final_project/ESP_song_files"

def get_ESP_file(filename):
	midi_song = ""
	with open("{}/{}".format(ESP_SONG_FILE_PATH, filename), "r") as f:
		midi_song = f.read()
	return midi_song

def request_handler(request):
	if (request["method"] == "GET"):
		filename = request['values']['song_file_name']
		return get_ESP_file(filename)
	else:
		return "Request not supported."



# response = request_handler({'method': 'GET', 'values': {'song_file_name': "less_i_know_the_better"}, 'args': ['song_file_name']})
# print(response)