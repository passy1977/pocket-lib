
use crate::pods::user::User;
use crate::traits::database_readable::DatabaseReadable;


impl DatabaseReadable<User, ()> for User {
    fn read(&self, _ : &()) -> Self {
        User::new()
    }
}