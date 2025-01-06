
use sqlite3::State;

use crate::pods::user::User;
use crate::traits::database_read::DatabaseRead;
use super::{Database, Status};


impl DatabaseRead<()> for User {
    fn read(&mut self, database: &Database, _ : &()) -> Status {

        if let Some(ref connection) = database.connection {
            if let Ok(ref mut statement) = connection.prepare("SELECT * FROM user;") {
                
                if let Ok(State::Row) = statement.next() {
                    self.uuid = statement.read(1).unwrap();
                    return Status::Ok
                } else {
                    return Status::Empty
                }
                
            }
        }

        Status::Error
    }
}
