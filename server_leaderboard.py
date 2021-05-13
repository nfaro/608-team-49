import sqlite3
import datetime
ht_db = '/var/jail/home/nfaro/leaderboards.db'

'''
The way the requests are going to work for the final project...

    ...for leaderboards for individual instrument combinations:

        GET requests (responds with current leaderboard):

            Getting a song's leaderboard (will show the top ten scores)

                - "action" : leaderboard

                - "song" : name of the song to retrieve

                - "instruments" : a string list representation of the instruments separated by plus signs
                                  i.e., "guitar+drums+bass"

        POST requests:

            Sending score to a leaderboard (responds with current leaderboard):

                - "action" : leaderboard

                - "name" : user name or group name

                - "song" : song to add to the leaderboard

                - "instruments" : a string list representation of the instruments separated by plus signs
                                  i.e., "guitar+drums+bass"

                - "score" : the raw score for the playing of a song (increasing difficulty
                            will have higher raw score because more notes are played
                            if they play them correctly)
'''




def request_handler(request):
    if (request["method"] == "POST"):
        try:
            action = request["form"]["action"]

            name = request['form']['name']
            # return "user done"
            song_name = request["form"]["song"]
            # return "item done"
            instruments = request["form"]["instruments"]
            # return "action done"
            score = int(request["form"]["score"])
            # return "value done"
        except Exception as e:
            # return e here or use your own custom return message for error catch
            #be careful just copy-pasting the try except as it stands since it will catch *all* Exceptions not just ones related to number conversion.
            return "Error: Missing one of the fields from the request are missing or not numbers"


        with sqlite3.connect(ht_db) as c:
            c.execute("""CREATE TABLE IF NOT EXISTS leaderboards (name text, song text, instruments text, score real);""")

            if (action == "leaderboard"):
                unupdated_leaderboard = c.execute("""SELECT * FROM leaderboards WHERE instruments=? AND song=? ORDER BY score DESC;""", (instruments, song_name)).fetchall()

                position_of_attempt = len(unupdated_leaderboard)
                i=0
                for i in range(len(unupdated_leaderboard)):
                    if (unupdated_leaderboard[i][3] < score):
                        break

                position_of_attempt = i - 1

                c.execute('''INSERT into leaderboards VALUES (?,?,?, ?);''', (name, song_name, instruments, score))

                unupdated_leaderboard = c.execute("""SELECT * FROM leaderboards WHERE instruments=? AND song=? ORDER BY score DESC;""", (instruments, song_name)).fetchall()

                string_to_return = "LEADERBOARD for " + song_name + " with " + instruments + ":\n"
                for i in range(min(10,len(unupdated_leaderboard))):
                    string_to_return += str(i+1) + ".) " + str(unupdated_leaderboard[i][0]) + " => " + str(unupdated_leaderboard[i][3]) + " points\n"

                if position_of_attempt > 9:
                    string_to_return += str(position_of_attempt + 1) + ".) " + name + " => " + str(score) + " points\n"

                return string_to_return
            return "Please Submit a correct POST request."

    else:
        try:
            # return request
            action = request["values"]["action"]
            # return "value done"
        except Exception as e:
            return "Error: Missing GET Headers Now!"
        with sqlite3.connect(ht_db) as c:
            c.execute("""CREATE TABLE IF NOT EXISTS leaderboards (name text, song text, instruments text, score real);""")
            if action == "leaderboard":
                try:
                    song_name = request["values"]["song"]
                    # return "item done"
                    instruments = request["values"]["instruments"]
                    # return "value done"
                except Exception as e:
                    return "Error: Missing GET Headers Now!"
                unupdated_leaderboard = c.execute("""SELECT * FROM leaderboards WHERE instruments=? AND song=? ORDER BY score DESC;""", (instruments, song_name)).fetchall()

                string_to_return = "LEADERBOARD for " + song_name + " with " + instruments + ":\n"
                for i in range(min(10,len(unupdated_leaderboard))):
                    string_to_return += str(i+1) + ".) " + str(unupdated_leaderboard[i][0]) + " => " + str(unupdated_leaderboard[i][3]) + " points\n"

                return string_to_return
            if action == "personalleaderboard":
                try:
                    # return request
                    action = request["values"]["action"]
                    # return "user done"
                    name = request["values"]["name"]
                    # return "value done"
                except Exception as e:
                    return "Error: Missing GET Headers Now!"

                leaderboard = c.execute("""SELECT * FROM leaderboards WHERE name=? ORDER BY score DESC;""", (name,)).fetchall()

                score_map = {}
                for i in leaderboard:
                    if (i[1] not in score_map.keys()):
                        score_map[i[1]] = i

                string_to_return = "HIGH SCORES FOR " + name + ":\n"
                for i in score_map.keys():
                    string_to_return += i + " on " + score_map[i][2] + " => " + str(score_map[i][3]) + " points\n"

                return string_to_return
            return "Please submit a correct GET request."
