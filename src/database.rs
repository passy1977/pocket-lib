use std::fs;


#[derive(Debug)]
pub struct Database {
    init: bool
}

impl Database {
    pub fn new() -> Self {
        Database {
            init:false
        }
    } 

    pub fn init(&mut self, file_db_path: String) {

        let mut database = Database {
            init:false
        };

        match fs::metadata(&file_db_path) {
            Ok(_) => {
                database.exist(&file_db_path);
            }
            Err(_) => {
                database.not_exist(&file_db_path);
            }
        }


    
    } 

    fn exist(&mut self, _file_db_path: &String) {
        todo!("Check file version")
    }

    fn not_exist(&mut self, _file_db_path: &String) {

    }

}