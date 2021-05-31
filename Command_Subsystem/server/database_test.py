from database import *



if __name__ == "__main__":
    database = "db/marsrover.db"
    conn = create_connection(database)
    create_rover_record
    # Test Obstacle Table
    obstacle_record_red = [["red", 1, 2, 1],
                           ["red", 3, 4, 1],
                           ["red", 5, 6, 3],
                           ["red", 7, 8, 1],
                           ["red", 9, 10, 1]]

    obstacle_record_green = [["green", 1, 2, 3],
                             ["green", 3, 4, 1],
                             ["green", 5, 6, 1],
                             ["green", 7, 8, 1],
                             ["green", 9, 10, 1]]

    obstacle_record_yellow = [["yellow", 1, 2, 1],
                              ["yellow", 3, 4, 1],
                              ["yellow", 5, 6, 2],
                              ["yellow", 7, 8, 1],
                              ["yellow", 9, 10, 1]]

    obstacle_record_blue = [["blue", 1, 2, 1],
                            ["blue", 3, 4, 1],
                            ["blue", 5, 6, 1],
                            ["blue", 7, 8, 4],
                            ["blue", 9, 10, 1]]

    obstacle_record_grey = [["grey", 1, 2, 1],
                            ["grey", 3, 4, 1],
                            ["grey", 5, 6, 1],
                            ["grey", 7, 8, 1],
                            ["grey", 9, 10, 1]]
    for i in [obstacle_record_red, obstacle_record_green, obstacle_record_blue, obstacle_record_grey, obstacle_record_yellow]:
        for j in i:
            create_obstacle_record(conn, j)
    


    print(select_top_obstacle(conn, 4, 1))


    print(select_all_obstacles(conn, 3))


    # Test Position Table