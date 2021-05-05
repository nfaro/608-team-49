import sqlite3
import datetime

rooms_db = '/var/jail/home/team49/room_info.db'

def request_handler(request):
    # lat = 0
    # lon = 0
    # reset = 0
    username = ''
    roomname = ''
    password = ''
    if request["method"] == "GET":
        try:
            username = request['values']['user']
        except Exception as e:
            # return e here or use your own custom return message for error catch
            #be careful just copy-pasting the try except as it stands since it will catch *all* Exceptions not just ones related to number conversion.
            return "Invalid username"
        if len(username) > 16 or len(username) < 1: return "Please provide a nonempty username with no more than 16 characters"
        conn = sqlite3.connect(rooms_db)
        c = conn.cursor()
        c.execute('''CREATE TABLE IF NOT EXISTS rooms_table (roomname text, password text, host text, player2 text, player3 text, player4 text, player5 text, ingame integer);''')
        # in_room = c.execute('''SELECT * FROM rooms_table WHERE user LIKE '%s';'''%(username)).fetchone()
        # if in_room:
        #     current_room = "Current room: " + in_room[0] + "\n"
        #     current_room += "Players:"
        #     for i in range(2,7):
        #         if in_room[i]: current_room += "\n" + in_room[i]
        #     conn.commit()
        #     conn.close()
        #     return current_room
        # else:
        all_rooms =  c.execute('''SELECT * FROM rooms_table''').fetchall()
        room_list = 'All rooms:'
        for room in all_rooms:
            if username in room:
                if room[7]:
                    return "Readying up..."
            is_full = True
            for i in range(2,7):
                if not room[i]: 
                    is_full = False
                if room[i] == username:
                    current_room = "Current room: " + room[0] + "\n"
                    current_room += "Players:"
                    for j in range(2,7):
                        if room[j]: 
                            current_room += "\n" + room[j]
                            if j == 2: 
                                current_room += " (Host)"
                    conn.commit()
                    conn.close()
                    return current_room
            room_list += "\n" + room[0]
            if room[1]: 
                room_list += " - Private"
            else:
                room_list += " - Public"
            if is_full: 
                room_list += " (FULL)"
        conn.commit()
        conn.close()
        if not all_rooms: return "No rooms to join..."
        return room_list

    elif request["method"] == "POST":
        try:
            username = request['form']['user']
            roomname = request['form']['roomname']
            password = request['form']['password']
            action = request['form']['action']
        except Exception as e:
            return "Invalid username, roomname, password, and/or action"
        if len(username) > 16 or len(username) < 1: return "Please provide a nonempty username with no more than 16 characters"
        if len(roomname) > 16 or len(roomname) < 1 : return "Please provide a nonempty room name with no more than 16 characters"
        if len(password) > 16: return "Please provide a password with no more than 16 characters"
        if len(action) < 1: return "Please provide a nonempty action"

        conn = sqlite3.connect(rooms_db)  # connect to that database (will create if it doesn't already exist)
        c = conn.cursor()  # move cursor into database (allows us to execute commands)
        c.execute('''CREATE TABLE IF NOT EXISTS rooms_table (roomname text, password text, host text, player2 text, player3 text, player4 text, player5 text, ingame integer);''')
        all_rooms =  c.execute('''SELECT * FROM rooms_table''').fetchall()
        if action == 'create':
            for room in all_rooms:
                if username in room:
                    return "You have already created a room"
            is_available = True
            for room in all_rooms:
                if room[0] == roomname:
                    is_available = False
                    break
            if is_available:
                c.execute('''INSERT into rooms_table VALUES (?,?,?,?,?,?,?,?);''', 
                         (roomname, password, username, None, None, None, None, 0))
                conn.commit()
                conn.close()
                pub_or_priv = 'private' if password else 'public'
                return "Sucessfully created " + pub_or_priv + " room " + roomname
            else:
                conn.commit()
                conn.close()
                return "Roomname is already taken"
        elif action == 'join':
            for room in all_rooms:
                if username in room:
                    return "You are already in a room"
            for room in all_rooms:
                if room[0] == roomname:
                    if not room[1] or password == room[1]:
                        room_copy = room
                        for i in range(2,7):
                            if not room_copy[i]:
                                room_copy = room_copy[:i] + (username,) + room_copy[i+1:]
                                c.execute('''DELETE FROM rooms_table WHERE roomname LIKE '%s';'''%(room_copy[0]))
                                c.execute('''INSERT into rooms_table VALUES (?,?,?,?,?,?,?,?);''', 
                                         (room_copy[0], room_copy[1], room_copy[2], room_copy[3], room_copy[4], room_copy[5], room_copy[6], room_copy[7]))
                                conn.commit()
                                conn.close()
                                return "Sucessfully joined " + room_copy[0]
                        conn.commit()
                        conn.close()
                        return room_copy[0] + " is full"
                    conn.commit()
                    conn.close()
                    return "Incorrect password"
            conn.commit()
            conn.close()
            return "Room name does not exist"
        elif action == 'leave':
            for room in all_rooms:
                if username in room:
                    room_copy = room
                    is_empty = True
                    is_host = False
                    delete_idx = 0
                    for i in range(2,7):
                        if room_copy[i] == username:
                            delete_idx = i
                            if i == 2: is_host = True
                        elif room_copy[i] != None:
                            is_empty = False
                    c.execute('''DELETE FROM rooms_table WHERE roomname LIKE '%s';'''%(room_copy[0]))
                    if is_empty:
                        conn.commit()
                        conn.close()
                        return "Left " + room_copy[0] + " (Room deleted)"
                    if is_host:
                        for i in range(3,7):
                            if room_copy[i]:
                                room_copy = (room_copy[0], room_copy[1]) + (room_copy[i],) + room_copy[3:i] + (None,) + room_copy[i+1:]
                                break
                    else:
                        room_copy = room_copy[:delete_idx] + (None,) + room_copy[delete_idx+1:]
                    c.execute('''INSERT into rooms_table VALUES (?,?,?,?,?,?,?,?);''', 
                         (room_copy[0], room_copy[1], room_copy[2], room_copy[3], room_copy[4], room_copy[5], room_copy[6], room_copy[7]))
                    conn.commit()
                    conn.close()
                    return "Left " + room_copy[0]
            conn.commit()
            conn.close() 
            return "You must be in a room in order to leave"
        elif action == 'ready':
            for room in all_rooms:
                if username in room:
                    if room[2] == username:
                        room_copy = room
                        c.execute('''DELETE FROM rooms_table WHERE roomname LIKE '%s';'''%(room_copy[0]))
                        c.execute('''INSERT into rooms_table VALUES (?,?,?,?,?,?,?,?);''', 
                        (room_copy[0], room_copy[1], room_copy[2], room_copy[3], room_copy[4], room_copy[5], room_copy[6], 1))                        
                        conn.commit()
                        conn.close()
                        return "Readying up..."
                    else:
                        return "Only the host can ready up"
            return "You must be the host of a room in order to ready up"  
        conn.commit()
        conn.close()   
        return "Please choose if you want to create, join, or leave a room"