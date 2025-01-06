
use std::path::MAIN_SEPARATOR_STR;

use crate::config::Config;
use crate::database::{Database, Status};
use crate::pods::device::Device;
use crate::pods::user::User;
use crate::utils::{Error::Message, Result};
use crate::traits::database_read::DatabaseRead;
use log::{ info, debug};

const APP_TAG: &str = "Session"; 

#[allow(unused)] 
#[derive(Debug)]
pub struct Session {
    database: Database,
    config: Option<Config>,
    device: Option<Device>,
    user: Option<User>,
    init: bool
}

#[allow(unused)] 
impl Session {
    pub fn new(config_json: Option<String>, config_path: Option<String>) -> Result<Session> {
        
        info!(target: APP_TAG, "Starting new session");

        match (config_json, config_path) {
            (json @ None, _) => Err(Message("Config json not defined")),
            (json, path) if !json.clone().unwrap().is_empty() => {

                let mut session = Session { 
                    database: Database::new(),
                    config: None,
                    device: None,
                    user: None,
                    init: false
                };

                match Config::new(path) {
                    Ok(conf) => {

                        match conf.parse(&json.unwrap().to_string()) {
                            Ok(device) => session.device = Some(device),
                            Err(msg) => return Err(msg)
                        }

                        session.config = Some(conf);

                        session.init = true;

                        if let Some(ref dev) = session.device {
                            info!(target: APP_TAG, "Session uuid:{}", dev.uuid);
                        }
                        
                        Ok(session)
                    }
                    Err(msg) => Err(msg)
                }

            },
            
            (_, _) => Err(Message("Un handled error"))
        }
    }

    pub fn init(&mut self) -> Result<()> {
        
        info!(target: APP_TAG, "Init session");

        match (&self.config, &self.device) {
            (Some(config), Some(device)) => {

                let mut file_db_path = config.config_path().clone();

                if !file_db_path.ends_with(MAIN_SEPARATOR_STR) {
                    file_db_path += MAIN_SEPARATOR_STR;
                }

                file_db_path += &device.uuid;
                file_db_path += ".db";

                debug!(target: APP_TAG, "file_db_path:{file_db_path}");

                if let Err(error) = self.database.init(file_db_path) {
                    match error {
                        Message(e) => return Err(Message(e))
                    }
                }

                
                self.user = Some(User::new());
    
                if self.user.clone().unwrap().read(&self.database, &()) == Status::Error {
                    self.user = None;
                    return Err(Message("User read error"))
                }

                Ok(())
            },
            (_, _) => Err(Message("Un handler error")) 
        }

    }


    pub fn register_device(&mut self, _email: &String, _passwd: &String, config_json: &String) -> Result<()> {
        Ok(())
    }

    pub fn database(&self) -> &Database {
        &self.database
    }

}



#[cfg(test)]
mod tests {
    use log::info;

    use crate::{database, pods::user::User, traits::database_read::DatabaseRead};

    use super::*;

    const APP_TAG: &str = "Session test"; 

    const REGISTRATION : &str = r#"
    {
        "uuid": "de10b648-9e9c-4b63-96df-2aa662fe20ba",
        "user_uuid": "1281d20e-4e6f-43d8-9aa5-099039f36206",
        "host": "127.0.0.1",
        "host_pub_key": "fookey"
    }
    "#;

    #[test]
    fn it_works() {
        info!(target: APP_TAG, "Start test");
        let s = Session::new(
            Option::Some(REGISTRATION.to_string()), 
            Option::None
        );


        assert!(s.is_ok());

        if let Ok(mut session) = s {
            
            if let Ok(()) = session.init() {
                if let Some(ref user) = session.user {
                    if user.is_empty() {
                        debug!(target: APP_TAG, "not logged");
                    } else {
                        debug!(target: APP_TAG, "logged");
                    }
                }
            }

            assert!(true);
        }

        assert!(false);

        
    }
}
