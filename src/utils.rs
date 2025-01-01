use std::fmt::{Display, Formatter};

#[derive(Debug)]
pub struct Error {
    pub message: &'static str
}

pub type Result<T> = std::result::Result<T, Error>;


impl Display for Error {
    fn fmt(&self, formatter: &mut Formatter) -> std::fmt::Result {
        self.message.fmt(formatter)
    }
}

impl std::error::Error for Error {
    fn description(&self) -> &str {
        self.message
    }
}