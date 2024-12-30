
pub enum Status {
    Unactive = 1,
    Active = 0,
    Deleted = 2,
    Invalidated = 3
}

pub struct Device {
    pub uuid: String,
    pub host: String,
    pub host_auth_user: String,
    pub host_auth_passwd: String,
    pub timestamp_last_update: u64,
    pub timestamp_creation: u64,
    pub status: Status
}

