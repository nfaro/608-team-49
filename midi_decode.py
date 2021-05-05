
import pretty_midi
# Load MIDI file into PrettyMIDI object

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
	    	if time in time_pitch:
	    		time_pitch[time].append(pitch)
	    	else:
	    		time_pitch[time] = [pitch]

	out = ""
	times = sorted(list(time_pitch.keys()))
	for time in times:
		out  += str(time) + "\n"
	 
	out+= "#"

	for time in times:
		section = ""
		for note in time_pitch[time]:
			section += str(note)


		out += section + '\n'
	
	with open("{}/{}".format(SAVE_FOLDER, output_filename), "w") as f:
		f.write(out)
	print(out)

path_to_file = input("Path to MIDI File: ")
out_path_name = input("Output Filename: ")
generate_ESP_file(path_to_file, out_path_name)





		