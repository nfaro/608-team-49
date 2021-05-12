
import pretty_midi
import os
# Load MIDI file into PrettyMIDI object

MIDI_FOLDER_NAME = "midi_files"
SAVE_FOLDER = "ESP_song_files"

BUTTON_1 = 60
BUTTON_2 = 61
BUTTON_3 = 62
BUTTON_4 = 63

def seconds_to_milliseconds(seconds):
	return int(seconds*1000)

def get_ESP_note(pitch):
	if pitch == BUTTON_1:
		return 1
	elif pitch == BUTTON_2:
		return 2
	elif pitch == BUTTON_3:
		return 3
	elif pitch == BUTTON_4:
		return 4
	else:
		raise Exception ("INVALID BUTTON INPUT! Pitch not recognized.")

def generate_ESP_file(path, output_filename):
	midi_data = pretty_midi.PrettyMIDI(path)

	time_pitch = {}

	for instrument in midi_data.instruments:
	    # Don't want to shift drum notes
	    for note in instrument.notes:
	    	time = seconds_to_milliseconds(note.start)
	    	pitch = get_ESP_note(note.pitch)
	    	duration = seconds_to_milliseconds(note.end - note.start)	    	
	    	if time in time_pitch:
	    		time_pitch[time]["pitch"].append(pitch)
	    	else:
	    		time_pitch[time] = {"pitch": [pitch], "duration": duration}

	out = ""
	times = sorted(list(time_pitch.keys()))
	for time in times:
		out  += str(time) + "\n"
	 
	out+= "#"

	for time in times:
		section = ""
		for note in time_pitch[time]["pitch"]:
			section += str(note)
		out += section + '\n'

	# out += "#"
	# for time in times:
	# 	section = str(time_pitch[time]["duration"])
	# 	out += section + '\n'
	
	with open("{}/{}".format(SAVE_FOLDER, output_filename), "w") as f:
		f.write(out)
# path_to_file = "midi_files/less_i_know_the_better.mid"
# out_path_name = "debug"
# path_to_file = input("Path to MIDI File: ")
# out_path_name = input("Output Filename: ")
# generate_ESP_file(path_to_file, out_path_name)

midi_filenames = os.listdir(MIDI_FOLDER_NAME) # returns list
for filename in midi_filenames:
	if filename == ".DS_Store":
		continue
	else:
		path_to_midi_file = "{}/{}".format(MIDI_FOLDER_NAME, filename)
		output_filename = filename[:-4]
		generate_ESP_file(path_to_midi_file, output_filename)






		