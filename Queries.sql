-- CREATE TABLE Users (
--     User_ID SERIAL PRIMARY KEY,
--     Full_Name VARCHAR(100) NOT NULL,
--     Email VARCHAR(100) UNIQUE NOT NULL,
--     Phone_Number VARCHAR(15),
--     Role VARCHAR(10) CHECK (Role IN ('Customer', 'Admin')) NOT NULL,
--     Password TEXT NOT NULL -- Assuming it's already encrypted before storage
-- );

-- CREATE TABLE Rooms (
--     Room_ID SERIAL PRIMARY KEY,
--     Room_Type VARCHAR(50) NOT NULL,
--     Price_Per_Night NUMERIC(10, 2) NOT NULL CHECK (Price_Per_Night >= 0),
--     Status VARCHAR(10) NOT NULL CHECK (Status IN ('Available', 'Booked')),
--     Description TEXT
-- );

-- CREATE TABLE Reservations (
--     Reservation_ID SERIAL PRIMARY KEY,
--     User_ID INT NOT NULL,
--     Room_ID INT NOT NULL,
--     Check_In_Date DATE NOT NULL,
--     Check_Out_Date DATE NOT NULL,
--     Total_Amount NUMERIC(10, 2) NOT NULL CHECK (Total_Amount >= 0),
--     Payment_Status VARCHAR(10) NOT NULL CHECK (Payment_Status IN ('Paid', 'Pending')),
--     Reservation_Status VARCHAR(10) NOT NULL CHECK (Reservation_Status IN ('Confirmed', 'Canceled')),
--     FOREIGN KEY (User_ID) REFERENCES Users(User_ID) ON DELETE CASCADE,
--     FOREIGN KEY (Room_ID) REFERENCES Rooms(Room_ID) ON DELETE CASCADE,
--     CHECK (Check_Out_Date > Check_In_Date)
-- );

-- CREATE TABLE Feedback (
--     Feedback_ID SERIAL PRIMARY KEY,
--     User_ID INT NOT NULL,
--     Reservation_ID INT NOT NULL,
--     Rating INT NOT NULL CHECK (Rating BETWEEN 1 AND 5),
--     Comments TEXT,
--     Submission_Date DATE NOT NULL DEFAULT CURRENT_DATE,
--     FOREIGN KEY (User_ID) REFERENCES Users(User_ID) ON DELETE CASCADE,
--     FOREIGN KEY (Reservation_ID) REFERENCES Reservations(Reservation_ID) ON DELETE CASCADE
-- );


-- INSERT INTO Users (Full_Name, Email, Phone_Number, Role, Password)
-- VALUES
-- ('Alice Johnson', 'alice@example.com', '1234567890', 'Customer', 'encrypted_pass1'),
-- ('Bob Smith', 'bob@example.com', '0987654321', 'Customer', 'encrypted_pass2'),
-- ('Carol Admin', 'carol.admin@example.com', '1122334455', 'Admin', 'encrypted_admin');

-- INSERT INTO Rooms (Room_Type, Price_Per_Night, Status, Description)
-- VALUES
-- ('Single', 75.00, 'Available', 'A cozy room with one single bed.'),
-- ('Double', 120.00, 'Booked', 'Spacious room with two beds.'),
-- ('Suite', 200.00, 'Available', 'Luxury suite with a view and mini-bar.');

-- INSERT INTO Reservations (User_ID, Room_ID, Check_In_Date, Check_Out_Date, Total_Amount, Payment_Status, Reservation_Status)
-- VALUES
-- (1, 2, '2025-05-10', '2025-05-12', 240.00, 'Paid', 'Confirmed'),
-- (2, 1, '2025-05-15', '2025-05-17', 150.00, 'Pending', 'Confirmed');

-- INSERT INTO Feedback (User_ID, Reservation_ID, Rating, Comments, Submission_Date)
-- VALUES
-- (1, 1, 5, 'Amazing stay. Clean rooms and friendly staff.', '2025-05-13'),
-- (2, 2, 4, 'Good experience, but room was a bit small.', '2025-05-14');


-- CREATE TABLE customers (
--     customer_id SERIAL PRIMARY KEY,
--     user_id INTEGER NOT NULL UNIQUE REFERENCES users(user_id) ON DELETE CASCADE,
--     loyalty_points INTEGER DEFAULT 0 CHECK (loyalty_points >= 0),
--     preferred_room_type VARCHAR(50),
--     last_stay_date DATE,
--     marketing_consent BOOLEAN DEFAULT FALSE
-- );

-- INSERT INTO customers (user_id)
-- SELECT user_id
-- FROM users
-- WHERE role = 'Customer';

ALTER TABLE reservations
ADD COLUMN payment_method VARCHAR(20),
ADD COLUMN payment_date DATE,
ADD COLUMN transaction_id VARCHAR(50),
ADD COLUMN payment_notes TEXT,
ADD CONSTRAINT reservations_payment_method_check 
    CHECK (payment_method IN ('Card', 'Cash', 'Online', 'Other'));

CREATE UNIQUE INDEX transaction_id_idx ON reservations(transaction_id);

ALTER TABLE reservations DROP CONSTRAINT reservations_payment_method_check;
ALTER TABLE reservations
ALTER COLUMN payment_method TYPE VARCHAR(100),
ALTER COLUMN payment_notes TYPE VARCHAR(100);
ALTER TABLE reservations
ADD CONSTRAINT reservations_payment_method_check
CHECK (payment_method IN ('Card', 'Cash', 'Online', 'Other', 'Online (Bank Transfer/Digital Wallet)'));
ALTER TABLE



ALTER TABLE users
ALTER COLUMN role SET DEFAULT 'user',
ALTER COLUMN role SET NOT NULL;
ALTER TABLE

ALTER TABLE users
DROP CONSTRAINT users_role_check;
