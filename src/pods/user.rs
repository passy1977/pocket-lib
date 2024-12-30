
pub enum Status {
    Unactive = 1,
    Active = 0,
    Deleted = 2
}

pub struct User {
    pub email: String,
    pub name: String,
    pub passwd: String,
    pub timestamp_last_update: u64,
    pub timestamp_creation: u64,
    pub status: Status
}
